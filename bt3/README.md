# SmartHome Vietnam – IoT Gateway: Tối ưu bộ nhớ struct & phân tích an toàn thiết bị

Mã nguồn: `main.c` (C11). Chương trình chỉ dùng `struct`, toán tử `.`, các kiểu cơ bản, câu lệnh điều kiện và vòng lặp trong `main()`. Không dùng mảng struct và không tự định nghĩa hàm.

---

## 1. Phân tích Input / Output

### 1.1. Input (nhập từ bàn phím)

| Dữ liệu | Thành phần struct | Kiểu lưu | Ràng buộc hợp lệ |
|---|---|---|---|
| Mã thiết bị | `deviceID` | `unsigned short` | 1 – 65535 |
| Loại thiết bị | `deviceType` | `unsigned char` | 1: Bình nóng lạnh, 2: Điều hòa, 3: Bếp từ |
| Trạng thái | `status` | `unsigned char` | Chỉ 0 (Tắt) hoặc 1 (Bật) |
| Điện áp U (V) | `voltage` | `double` | U > 0 |
| Dòng điện I (A) | `current` | `float` | I ≥ 0 |
| Công suất đo từ thiết bị P (W) | `measuredPower` | `double` | P ≥ 0 |
| Thời gian hoạt động t (phút) | `operatingMinutes` | `int` | t ≥ 0 |

Nếu một giá trị nhập vào không phải số (`scanf` trả về khác 1), chương trình cũng xem đó là dữ liệu rác.

### 1.2. Output

1. **Bảng so sánh bộ nhớ**: `sizeof` của struct chưa tối ưu và hai giải pháp. Mỗi dòng ghi rõ số byte dữ liệu thật và số byte padding.
2. **Thông báo lỗi và dừng tính toán** khi dữ liệu không hợp lệ. Chương trình liệt kê từng trường bị lỗi.
3. **Báo cáo an toàn thiết bị**:
   - P đo, P tính `U × I` và % lệch giữa hai giá trị;
   - các cảnh báo: *Cảm biến công suất báo sai lệch*, *QUÁ TẢI NGUY HIỂM*, *RÒ RỈ ĐIỆN HOẶC TẢI ẨN*;
   - điện năng tiêu thụ (kWh).

Các cờ cảnh báo và kết quả kWh cũng được ghi lại vào chính struct: `isPowerMismatch`, `isOverloaded`, `isLeaking`, `energyKWh`.

---

## 2. Struct hiện tại & vấn đề Memory Padding

Đề bài không cho sẵn struct gốc. Dưới đây là struct mô phỏng cách khai báo "tự phát": các trường được thêm vào theo thứ tự nghĩ ra, xen kẽ kiểu 1 byte với kiểu 8 byte.

```c
struct DeviceUnoptimized {
    unsigned char  status;
    double         voltage;
    unsigned char  deviceType;
    float          current;
    unsigned char  isOverloaded;
    double         measuredPower;
    unsigned short deviceID;
    int            operatingMinutes;
    unsigned char  isLeaking;
    double         energyKWh;
    unsigned char  isPowerMismatch;
};
```

**Quy tắc căn chỉnh (Data Alignment):**

- Mỗi trường phải nằm ở địa chỉ là bội số của kích thước kiểu của nó: `double` ở bội của 8, `float`/`int` ở bội của 4, `short` ở bội của 2.
- Kích thước toàn struct phải là bội số của trường có yêu cầu căn chỉnh lớn nhất (ở đây là 8).

Khi không thỏa, trình biên dịch tự chèn thêm byte đệm (padding).

**Sơ đồ bộ nhớ (GCC x86-64; ARM Cortex-M theo chuẩn AAPCS cũng căn `double` 8 byte nên ra cùng kết quả):**

| Offset | Trường | Kích thước | Padding sau trường |
|---|---|---|---|
| 0 | `status` | 1 | **7** |
| 8 | `voltage` | 8 | 0 |
| 16 | `deviceType` | 1 | **3** |
| 20 | `current` | 4 | 0 |
| 24 | `isOverloaded` | 1 | **7** |
| 32 | `measuredPower` | 8 | 0 |
| 40 | `deviceID` | 2 | **2** |
| 44 | `operatingMinutes` | 4 | 0 |
| 48 | `isLeaking` | 1 | **7** |
| 56 | `energyKWh` | 8 | 0 |
| 64 | `isPowerMismatch` | 1 | **7** (đệm cuối) |
| | **Tổng** | **39 byte dữ liệu** | **33 byte padding → sizeof = 72** |

Gần **46%** dung lượng của mỗi struct là byte đệm vô nghĩa. Mỗi thiết bị Gateway quản lý đều tốn thêm 33 byte RAM. Với nhiều thiết bị và nhiều bản sao dữ liệu (buffer gửi lên cloud, lịch sử đo), phần lãng phí này dồn lại và gây tràn bộ nhớ.

---

## 3. Đề xuất giải pháp tái cấu trúc

### Giải pháp 1 – Flat Struct Optimization (sắp xếp kích thước giảm dần)

Giữ struct phẳng nhưng sắp xếp các trường theo thứ tự `double` (8) → `float`/`int` (4) → `short` (2) → `char` (1). Mỗi trường khi đó đều tự rơi vào địa chỉ đã căn chỉnh, nên không cần chèn padding ở giữa.

```c
struct DeviceOptimized {
    double         voltage;          /* offset 0  */
    double         measuredPower;    /* offset 8  */
    double         energyKWh;        /* offset 16 */
    float          current;          /* offset 24 */
    int            operatingMinutes; /* offset 28 */
    unsigned short deviceID;         /* offset 32 */
    unsigned char  deviceType;       /* offset 34 */
    unsigned char  status;           /* offset 35 */
    unsigned char  isOverloaded;     /* offset 36 */
    unsigned char  isLeaking;        /* offset 37 */
    unsigned char  isPowerMismatch;  /* offset 38 */
};                                   /* 39 byte + 1 byte đệm cuối = 40 */
```

→ **sizeof = 40 byte, chỉ 1 byte padding** (bắt buộc phải có, để kích thước là bội của 8).

### Giải pháp 2 – Nested Struct (nhóm theo chức năng)

Tách thành các struct con theo vai trò: số liệu điện, thông tin vận hành, cờ cảnh báo. Trong mỗi struct con, các trường vẫn được sắp xếp giảm dần.

```c
struct ElectricalReading { double voltage; double measuredPower; float current; };            /* 20 + 4 đệm = 24 */
struct OperationInfo     { double energyKWh; int operatingMinutes; unsigned short deviceID;
                           unsigned char deviceType; unsigned char status; };                   /* 16 */
struct SafetyFlags       { unsigned char isOverloaded, isLeaking, isPowerMismatch; };           /* 3 */

struct DeviceNested {
    struct ElectricalReading reading;  /* offset 0  – 24 byte */
    struct OperationInfo     info;     /* offset 24 – 16 byte */
    struct SafetyFlags       flags;    /* offset 40 – 3 byte, +5 đệm cuối */
};
```

→ **sizeof = 48 byte, 9 byte padding.**

Nguyên nhân phát sinh padding:

- Mỗi struct con phải tự làm tròn kích thước theo căn chỉnh của riêng nó. Ví dụ `ElectricalReading` có 20 byte dữ liệu nhưng phải đệm lên 24.
- Trình biên dịch **không được** đặt trường của struct con khác vào phần đệm cuối này.

Vì vậy struct lồng nhau luôn có nguy cơ tốn padding hơn struct phẳng chứa cùng các trường.

### Kết quả đo thực tế (chương trình tự in ra)

```
Struct chua toi uu (tu phat)       sizeof =  72 byte | du lieu = 39 | padding = 33
Giai phap 1: Flat (giam dan)       sizeof =  40 byte | du lieu = 39 | padding = 1
Giai phap 2: Nested (long nhau)    sizeof =  48 byte | du lieu = 39 | padding = 9
Giai phap 1 tiet kiem 32 byte/thiet bi (44.4%) so voi struct cu.
```

Số byte dữ liệu được tính bằng tổng `sizeof` của từng thành viên, ví dụ `sizeof(dev.voltage) + ...`. Nhờ vậy, con số này vẫn đúng nếu chạy trên kiến trúc khác.

---

## 4. So sánh & lựa chọn

| Tiêu chí | Struct cũ | Giải pháp 1 – Flat (giảm dần) | Giải pháp 2 – Nested |
|---|---|---|---|
| **sizeof (x86-64 / Cortex-M)** | 72 byte | **40 byte (−44,4%)** | 48 byte (−33,3%) |
| **Padding** | 33 byte | **1 byte** | 9 byte |
| **RAM cho 100 thiết bị** | 7.200 byte | **4.000 byte** | 4.800 byte |
| **Độ phức tạp truy xuất** | 1 cấp: `dev.voltage` | **1 cấp: `dev.voltage`** | 2 cấp: `dev.reading.voltage` – dòng lệnh dài hơn, dễ gõ nhầm nhóm |
| **Tính dễ bảo trì** | Kém – thứ tự lộn xộn | Trung bình – các trường xếp theo kích thước nên trường liên quan có thể nằm xa nhau; bù lại bằng comment chia nhóm `/* 8 byte */`, `/* 4 byte */`… | **Tốt** – đọc là biết trường thuộc nhóm chức năng nào; có thể sao chép nguyên một nhóm (`a.reading = b.reading;`) |
| **Khả năng mở rộng** | Thêm trường dễ làm padding tệ hơn | Thêm trường phải chèn đúng nhóm kích thước, nếu không sẽ phát sinh padding mới | **Tốt** – thêm hẳn một nhóm mới (ví dụ `struct NetworkInfo`) mà không đụng các nhóm cũ; nhưng mỗi nhóm mới có thể thêm đệm cuối riêng |

### Lựa chọn: **Giải pháp 1 – Flat Struct Optimization**

Lý do chọn giải pháp này cho IoT Gateway:

1. **RAM là tài nguyên khan hiếm nhất trên vi điều khiển.** Bài toán gốc chính là tràn RAM.
   - Giải pháp 1 đạt mức tối thiểu lý thuyết: 40 byte, chỉ 1 byte đệm bắt buộc.
   - So với Giải pháp 2, mỗi thiết bị tiết kiệm thêm 8 byte (17%).
2. **Truy xuất một cấp `dev.truong`** cho mã ngắn, dễ đọc trong các biểu thức an toàn. Ví dụ: `dev.status == 0 && dev.current > NGUONG_RO_RI`.
3. **Không cần chỉ thị đặc thù trình biên dịch.** Phương án ép bỏ padding bằng `#pragma pack(1)` hoặc `__attribute__((packed))` cho 39 byte, nhưng có hai nhược điểm:
   - nằm ngoài phạm vi bài học;
   - gây truy cập bộ nhớ không căn chỉnh. Trên một số lõi ARM, ví dụ Cortex-M0, truy cập `double`/`float` lệch địa chỉ có thể gây lỗi HardFault hoặc chạy chậm.

   Sắp xếp lại thứ tự trường là cách tối ưu an toàn và chạy được trên mọi nền tảng.
4. Nhược điểm về bảo trì được khắc phục bằng **comment chia nhóm kích thước** và **quy ước**: khi thêm trường mới, phải chèn đúng nhóm kích thước của nó.

---

## 5. Thiết kế luồng xử lý (giải pháp đã chọn)

### 5.1. Mã giả

```
BẮT ĐẦU
  // Phần 1 – So sánh bộ nhớ
  IN sizeof, tổng byte dữ liệu, padding của: DeviceUnoptimized, DeviceOptimized, DeviceNested

  // Phần 2 – Nhập và kiểm tra ngay
  NHẬP deviceID      → nếu không phải số hoặc ngoài [1, 65535]: BÁO LỖI, DỪNG
  NHẬP deviceType    → nếu ∉ {1, 2, 3}: BÁO LỖI, DỪNG
  NHẬP status        → nếu ∉ {0, 1}: BÁO "dữ liệu không hợp lệ", DỪNG
  NHẬP U, I, P, t    → nếu scanf thất bại: đánh dấu duLieuLoi
  (sau mỗi scanf: while (getchar() != '\n'); để làm sạch bộ đệm)

  // Phần 3 – Lọc dữ liệu rác (kiểm tra tất cả rồi mới dừng, để liệt kê đủ lỗi)
  NẾU U ≤ 0  → báo lỗi U
  NẾU I < 0  → báo lỗi I
  NẾU P < 0  → báo lỗi P
  NẾU t < 0  → báo lỗi t
  NẾU có bất kỳ lỗi nào → IN "DỪNG TÍNH TOÁN", KẾT THÚC

  // Phần 4 – Phân tích an toàn (truy xuất bằng toán tử .)
  dev.isOverloaded = dev.isLeaking = dev.isPowerMismatch = 0
  pCalc = dev.voltage × (double)dev.current
  NẾU pCalc > 0:
      lech% = |dev.measuredPower − pCalc| / pCalc × 100
      NẾU lech% > 5 (+ epsilon) → dev.isPowerMismatch = 1
  NGƯỢC LẠI (I = 0):
      NẾU dev.measuredPower > 0 → dev.isPowerMismatch = 1
  NẾU dev.current > 30.0 HOẶC dev.measuredPower > 6600 HOẶC pCalc > 6600
      → dev.isOverloaded = 1
  NẾU dev.status == 0 VÀ dev.current > 0.05
      → dev.isLeaking = 1
  dev.energyKWh = dev.measuredPower × ((double)dev.operatingMinutes / 60.0) / 1000.0

  // Phần 5 – Xuất báo cáo
  IN thông số, P đo / P tính / % lệch, từng cảnh báo (hoặc "An toàn"), kWh
KẾT THÚC
```

### 5.2. Các quyết định xử lý biên

| Tình huống | Cách xử lý | Lý do |
|---|---|---|
| Chia `t / 60` | `(double)dev.operatingMinutes / 60.0` | Nếu chia nguyên, `59 / 60 = 0` và mất toàn bộ điện năng. Với cách chia thực, t = 59 phút ra đúng 2.1633 kWh ở 2200 W |
| So sánh `dev.current` (`float`) với ngưỡng | Hằng ngưỡng khai báo kiểu `float` (`0.05f`, `30.0f`) | **Lỗi phát hiện khi kiểm thử:** `0.05f` thực tế là 0.0500000007. Nếu so với `double 0.05`, chương trình báo rò rỉ sai khi I = 0.05 A (đúng ngưỡng, lẽ ra không cảnh báo). So sánh cùng kiểu thì đúng |
| % lệch đúng 5,00% | Thêm `EPSILON = 1e-9` | Phép chia số thực có thể ra 5.0000000001 do làm tròn, gây cảnh báo sai |
| I = 0 nên P_calc = 0 (không chia được) | Báo sai lệch nếu P đo > 0 | Tránh chia cho 0. Không có dòng điện mà thiết bị vẫn báo có công suất là dấu hiệu cảm biến sai |
| Quá tải: P nào? | Xét cả P đo **và** P tính | Ưu tiên an toàn: nếu cảm biến công suất hỏng (báo thấp), vẫn phát hiện quá tải nhờ U × I |
| kWh dùng P nào? | P đo (`measuredPower`) | Theo đúng công thức đề bài. Nếu P đo lệch, cảnh báo sai lệch đã được hiển thị kèm |
| Thiết bị TẮT nhưng có dòng | Vẫn tính kWh | Đây chính là điện năng thất thoát do rò rỉ hoặc tải ẩn, cần ghi nhận |

---

## 6. Kiểm thử

Biên dịch `gcc -std=c11 -Wall -Wextra -pedantic`: không có cảnh báo. Kết quả chạy thực tế:

| # | Trường hợp | Input (status, U, I, P, t) | Kết quả |
|---|---|---|---|
| TC01 | Bình thường | 1, 220, 10, 2200, 90 | An toàn; 3.3000 kWh |
| TC02 | Cảm biến lệch > 5% | 1, 220, 10, **2400**, 60 | Lệch 9.09% → *Cảm biến công suất báo sai lệch* |
| TC03 | Biên lệch đúng 5% | 1, 220, 10, 2310, 60 | Lệch 5.00% → **không** cảnh báo |
| TC04 | Biên lệch 5,01% | 1, 220, 10, 2310.3, 60 | → *sai lệch* |
| TC05 | Quá tải dòng | 1, 220, **35**, 7700, 30 | *QUÁ TẢI NGUY HIỂM* (I và P đều vượt) |
| TC06 | Quá tải công suất, dòng < 30 A | 1, 231, 29, **6699**, 30 | *QUÁ TẢI NGUY HIỂM* (chỉ P vượt) |
| TC07 | Biên 30 A / 6600 W | 1, 220, 30, 6600, 30 | **Không** quá tải (điều kiện là `>`) |
| TC08 | Vừa vượt biên | 1, 220, 30.01, 6602.2, 30 | *QUÁ TẢI NGUY HIỂM* |
| TC09 | Rò rỉ | **0**, 220, 0.2, 44, 600 | *RÒ RỈ ĐIỆN HOẶC TẢI ẨN*; 0.44 kWh thất thoát |
| TC10 | Biên rò rỉ 0.05 A | 0, 220, 0.05, 11, 600 | **Không** cảnh báo (sau khi sửa lỗi so sánh float/double) |
| TC11 | Vừa vượt biên rò rỉ | 0, 220, 0.06, 13.2, 600 | *RÒ RỈ ĐIỆN HOẶC TẢI ẨN* |
| TC12 | I = 0 nhưng P > 0 | 1, 220, 0, 50, 10 | *Sai lệch* (không chia cho 0) |
| TC13 | Chia số thực | 1, 220, 10, 2200, **59** | 2.1633 kWh (chia nguyên sẽ ra 0) |
| TC14 | U = 0 | 1, **0**, 5, 0, 10 | Lỗi cảm biến → **dừng tính toán** |
| TC15 | I âm và t âm | 1, 220, **−1**, 100, **−5** | Liệt kê cả 2 lỗi → **dừng** |
| TC16 | U nhập chữ | 1, **abc**, 5, 100, 10 | Báo nhập không phải số + lỗi U → **dừng** |
| TC17 | Trạng thái sai | status = **2** | *Dữ liệu không hợp lệ* → **dừng** |

---

## 7. Biên dịch & chạy

```bash
gcc -std=c11 -Wall -o main main.c
./main          # Windows: main.exe
```

Giá trị `sizeof` phụ thuộc kiến trúc. Trên x86-64 và ARM Cortex-M (32-bit) ra 72 / 40 / 48 byte như trên. Trên vi điều khiển 8-bit như AVR, các kiểu gần như không cần căn chỉnh nên padding rất nhỏ.

Chương trình tự tính và in số byte dữ liệu, số byte padding cho nền tảng đang chạy.
