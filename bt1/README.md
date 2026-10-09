# Smart Home IoT – Sửa lỗi hiển thị công suất khi thiết bị bị ngắt điện

## 1. Mô tả sự cố

Bình nóng lạnh (ID 101) nhảy aptomat khi dòng điện tăng đột biến lên **35.0 A**. Ứng dụng báo trạng thái **TẮT**, nhưng chỉ số công suất vẫn hiển thị **7700.00 W**.

Theo quy tắc nghiệp vụ, thiết bị ở trạng thái TẮT phải có `power_watt = 0.0 W`.

## 2. Phân tích lỗi

### 2.1. Dòng lệnh sai logic

Trong `main.c` gốc, lỗi nằm ở **thứ tự hai khối lệnh** (dòng 23 và khối `if` ở dòng 26–29):

```c
// Dòng 23 – tính công suất khi CHƯA kiểm tra an toàn
heater.power_watt = heater.voltage * heater.current;

// Dòng 26–29 – kiểm tra quá tải và ngắt điện SAU khi đã tính công suất
if (heater.current > 30.0f) {
    heater.is_overloaded = 1;
    heater.is_active = 0;   // ngắt điện nhưng KHÔNG cập nhật lại power_watt
}
```

### 2.2. Nguyên nhân kỹ thuật

1. **Sai thứ tự cập nhật thuộc tính của struct.**
   - `power_watt` được tính ngay từ đầu bằng `220.0 × 35.0 = 7700.0`.
   - Sau đó khối `if` đổi `is_active` từ 1 về 0, nhưng không ghi lại `power_watt`.
   - Các thành viên của struct là các ô nhớ độc lập. Đổi `is_active` **không** làm `power_watt` tự thay đổi theo.
   - Kết quả: struct ở trạng thái mâu thuẫn, `is_active = 0` trong khi `power_watt = 7700.0`.
2. **Công suất không phụ thuộc vào trạng thái bật/tắt.**
   - Công thức `P = V × I` được áp dụng vô điều kiện.
   - Vì vậy, ngay cả khi thiết bị đã TẮT sẵn (`is_active = 0`), cảm biến ghi nhận dòng rò hoặc nhiễu (ví dụ 0.5 A) vẫn sinh ra công suất khác 0 (110.00 W).
   - Điều này vi phạm quy tắc: thiết bị TẮT thì công suất bắt buộc bằng 0.0 W.
3. **Điều kiện quá tải không xét trạng thái thiết bị.**
   - Chương trình kiểm tra `current > 30.0f` mà không kiểm tra thiết bị có đang BẬT hay không.
   - Theo nghiệp vụ, việc ngắt aptomat chỉ áp dụng khi thiết bị đang BẬT.

### 2.3. Hướng khắc phục

Tổ chức lại luồng xử lý theo đúng thứ tự nghiệp vụ:

1. **Kiểm tra an toàn trước.** Nếu thiết bị đang BẬT và `current > 30.0 A`, thì đặt `is_overloaded = 1` và `is_active = 0`.
2. **Tính công suất sau, dựa trên trạng thái cuối cùng.** Nếu `is_active == 1` thì `power_watt = voltage × current`; ngược lại `power_watt = 0.0f`.

Ngoài ra, bản sửa còn có hai thay đổi nhỏ:

- Khởi tạo `heater.power_watt = 0.0f` để không bao giờ đọc phải giá trị rác.
- Sửa lỗi chính tả trên tiêu đề in ra: "HETHONG" thành "HE THONG".

Bản sửa chỉ dùng `struct`, `if/else` và `printf`, nằm trong phạm vi kiến thức của buổi học.

## 3. Bảng Test Cases đối chứng

| # | Trường hợp kiểm thử | Dữ liệu đầu vào | Kết quả sai thực tế (mã gốc) | Kết quả đúng mong đợi (mã đã sửa) |
|---|---|---|---|---|
| TC01 | Thiết bị đang BẬT, dòng điện vượt ngưỡng → tự động ngắt điện | `voltage = 220.0`, `current = 35.0`, `is_active = 1`, `is_overloaded = 0` | Trạng thái: **TAT**<br>Công suất: **7700.00 W** ❌<br>Cảnh báo: CO (DA NGAT DIEN) | Trạng thái: **TAT**<br>Công suất: **0.00 W** ✅<br>Cảnh báo: CO (DA NGAT DIEN) |
| TC02 | Thiết bị đang TẮT, cảm biến ghi nhận dòng rò/nhiễu | `voltage = 220.0`, `current = 0.5`, `is_active = 0`, `is_overloaded = 0` | Trạng thái: **TAT**<br>Công suất: **110.00 W** ❌<br>Cảnh báo: KHONG | Trạng thái: **TAT**<br>Công suất: **0.00 W** ✅<br>Cảnh báo: KHONG |

> **Kiểm tra hồi quy:** với thiết bị BẬT, `current = 20.0 A` cho kết quả `BAT – 4400.00 W – KHONG`. Tại đúng ngưỡng `current = 30.0 A` cho kết quả `BAT – 6600.00 W – KHONG` (vì điều kiện là `> 30.0`, không phải `>= 30.0`). Cả mã gốc và mã đã sửa cho kết quả giống nhau ở các trường hợp này, tức bản sửa không làm hỏng luồng hoạt động bình thường.

## 4. Biên dịch và chạy

```bash
gcc -std=c11 -Wall -o main main.c
./main          # Windows: main.exe
```

Kết quả với dữ liệu mặc định (TC01):

```
=== HE THONG GIAM SAT THIET BI SMART HOME ===
Ma thiet bi: 101
Trang thai hoat dong: TAT
Cong suat tieu thu: 0.00 W
Canh bao qua tai (>30A): CO (DA NGAT DIEN)
```

Để chạy TC02, sửa phần khởi tạo trong `main.c` thành `heater.current = 0.5f;` và `heater.is_active = 0;`, rồi biên dịch lại.
