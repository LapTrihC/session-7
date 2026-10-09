/*
 * RIKKEI SMARTHOME - Smart Gateway: dieu khien trung tam thiet bi dien
 * Chuan C11. Bien dich: gcc -std=c11 -Wall -o main main.c
 *
 * Pham vi ky thuat: chi dung ham main(), khong mang cau truc,
 * truy xuat thuoc tinh struct thuan tuy qua toan tu '.'.
 *
 * Thu tu xu ly nghiep vu:
 *   0. Kiem tra hop le du lieu (cam bien am -> huy xu ly; gio van hanh ngoai (0, 744] -> tu choi tinh hoa don)
 *   1. An toan dien: dong dien > 30.0 A -> ngat khan cap (status = 0)
 *   2. Tiet kiem nang luong: dang bat va vang nguoi >= 15 phut -> tu dong tat
 *   3. Tinh kWh = ratedPowerKW * operatingHours va tien dien bac thang EVN (chua VAT)
 */
#include <stdio.h>
#include <string.h>

struct SmartDevice {
    int   deviceID;         /* Ma dinh danh thiet bi */
    char  deviceName[50];   /* Ten thiet bi */
    int   status;           /* 1: dang bat, 0: da tat */
    float ratedPowerKW;     /* Cong suat dinh muc (kW) */
};

struct EnergySensor {
    float voltageVolts;         /* Dien ap do duoc (V) */
    float currentAmperes;       /* Dong dien do duoc (A) */
    int   unoccupancyMinutes;   /* Thoi gian vang nguoi (phut) */
};

int main(void) {
    /* ---------------- Hang so nghiep vu ---------------- */
    const float NGUONG_DONG_DIEN = 30.0f;   /* A */
    const int   NGUONG_VANG_NGUOI = 15;     /* phut */
    const float GIO_TOI_DA_THANG = 744.0f;  /* 31 ngay x 24 gio */

    const double GIA_BAC_1 = 1806.0;        /* 0   - 50  kWh */
    const double GIA_BAC_2 = 1866.0;        /* 51  - 100 kWh */
    const double GIA_BAC_3 = 2167.0;        /* 101 - 200 kWh */
    const double GIA_BAC_4 = 2729.0;        /* tu 201 kWh */

    /* ---------------- Bien cau truc va bien phu tro ---------------- */
    struct SmartDevice  device;
    struct EnergySensor sensor;

    float  operatingHours;          /* So gio van hanh trong thang (nguoi dung nhap) */
    int    statusBanDau;            /* Luu trang thai truoc khi ap dung luat tu dong */
    int    canhBaoQuaTai = 0;       /* 1: da ngat khan cap do qua tai */
    int    daTuDongTat = 0;         /* 1: da tu tat do vang nguoi */
    int    hoaDonHopLe = 1;         /* 0: tu choi tinh hoa don */

    double kWh = 0.0, tienDien = 0.0;
    double kWhBac1 = 0.0, kWhBac2 = 0.0, kWhBac3 = 0.0, kWhBac4 = 0.0;
    size_t len;

    /* Bien dinh dang tien co dau cham hang nghin */
    long long soTien;
    char chuoiSo[32], chuoiTien[40];
    int  doDai, i, k;

    printf("================ RIKKEI SMARTHOME - SMART GATEWAY ================\n");

    /* ======================= NHAP DU LIEU ======================= */
    printf("\n--- THONG TIN THIET BI (SmartDevice) ---\n");

    printf("Ma thiet bi (deviceID): ");
    if (scanf("%d", &device.deviceID) != 1) {
        printf("LOI: Ma thiet bi phai la so nguyen. Huy thao tac xu ly!\n");
        return 1;
    }
    while (getchar() != '\n');      /* lam sach bo dem truoc khi dung fgets */

    printf("Ten thiet bi (deviceName): ");
    if (fgets(device.deviceName, sizeof(device.deviceName), stdin) == NULL) {
        device.deviceName[0] = '\0';
    }
    len = strlen(device.deviceName);
    if (len > 0 && device.deviceName[len - 1] == '\n') {
        device.deviceName[len - 1] = '\0';
    } else if (len == sizeof(device.deviceName) - 1) {
        while (getchar() != '\n');  /* ten qua dai: bo phan thua */
    }
    if (strlen(device.deviceName) == 0) {
        printf("LOI: Ten thiet bi khong duoc de trong. Huy thao tac xu ly!\n");
        return 1;
    }

    printf("Trang thai hien tai (1: Bat, 0: Tat): ");
    if (scanf("%d", &device.status) != 1 || (device.status != 0 && device.status != 1)) {
        printf("LOI: Trang thai chi nhan gia tri 0 hoac 1. Huy thao tac xu ly!\n");
        return 1;
    }
    while (getchar() != '\n');

    printf("Cong suat dinh muc (kW): ");
    if (scanf("%f", &device.ratedPowerKW) != 1 || device.ratedPowerKW <= 0.0f) {
        printf("LOI: Cong suat dinh muc phai la so thuc lon hon 0. Huy thao tac xu ly!\n");
        return 1;
    }
    while (getchar() != '\n');

    printf("\n--- DU LIEU CAM BIEN (EnergySensor) ---\n");

    printf("Dien ap do duoc (V): ");
    if (scanf("%f", &sensor.voltageVolts) != 1) {
        printf("LOI: Du lieu dien ap khong phai so. Huy thao tac xu ly!\n");
        return 1;
    }
    while (getchar() != '\n');

    printf("Dong dien do duoc (A): ");
    if (scanf("%f", &sensor.currentAmperes) != 1) {
        printf("LOI: Du lieu dong dien khong phai so. Huy thao tac xu ly!\n");
        return 1;
    }
    while (getchar() != '\n');

    printf("Thoi gian vang nguoi (phut): ");
    if (scanf("%d", &sensor.unoccupancyMinutes) != 1) {
        printf("LOI: Thoi gian vang nguoi phai la so nguyen. Huy thao tac xu ly!\n");
        return 1;
    }
    while (getchar() != '\n');

    printf("\n--- THONG TIN VAN HANH ---\n");
    printf("So gio van hanh trong thang (0 < gio <= 744): ");
    if (scanf("%f", &operatingHours) != 1) {
        operatingHours = -1.0f;     /* khong phai so -> danh dau khong hop le */
    }
    while (getchar() != '\n');

    /* ============ KIEM TRA LOI DU LIEU 1: CAM BIEN BAT THUONG ============ */
    if (sensor.voltageVolts < 0.0f || sensor.currentAmperes < 0.0f
        || sensor.unoccupancyMinutes < 0) {
        printf("\n[LOI PHAN CUNG CAM BIEN] Phat hien thong so am:\n");
        if (sensor.voltageVolts < 0.0f) {
            printf("  - Dien ap = %.2f V (< 0)\n", sensor.voltageVolts);
        }
        if (sensor.currentAmperes < 0.0f) {
            printf("  - Dong dien = %.2f A (< 0)\n", sensor.currentAmperes);
        }
        if (sensor.unoccupancyMinutes < 0) {
            printf("  - Thoi gian vang nguoi = %d phut (< 0)\n", sensor.unoccupancyMinutes);
        }
        printf("=> Huy thao tac xu ly. Vui long kiem tra lai cam bien!\n");
        return 1;
    }

    /* ============ KIEM TRA LOI DU LIEU 2: GIO VAN HANH ============ */
    if (operatingHours <= 0.0f || operatingHours > GIO_TOI_DA_THANG) {
        hoaDonHopLe = 0;
    }

    /* ======================= AP DUNG LUAT TU DONG ======================= */
    statusBanDau = device.status;

    /* Uu tien 1: An toan dien - qua tai dong dien */
    if (sensor.currentAmperes > NGUONG_DONG_DIEN) {
        device.status = 0;
        canhBaoQuaTai = 1;
    }

    /* Uu tien 2: Tiet kiem nang luong - phong trong >= 15 phut */
    if (device.status == 1 && sensor.unoccupancyMinutes >= NGUONG_VANG_NGUOI) {
        device.status = 0;
        daTuDongTat = 1;
    }

    /* ======================= TINH DIEN NANG & CHI PHI ======================= */
    if (hoaDonHopLe) {
        kWh = (double)device.ratedPowerKW * operatingHours;

        if (kWh <= 50.0) {
            kWhBac1 = kWh;
        } else if (kWh <= 100.0) {
            kWhBac1 = 50.0;
            kWhBac2 = kWh - 50.0;
        } else if (kWh <= 200.0) {
            kWhBac1 = 50.0;
            kWhBac2 = 50.0;
            kWhBac3 = kWh - 100.0;
        } else {
            kWhBac1 = 50.0;
            kWhBac2 = 50.0;
            kWhBac3 = 100.0;
            kWhBac4 = kWh - 200.0;
        }

        tienDien = kWhBac1 * GIA_BAC_1 + kWhBac2 * GIA_BAC_2
                 + kWhBac3 * GIA_BAC_3 + kWhBac4 * GIA_BAC_4;
    }

    /* ======================= XUAT BAO CAO ======================= */
    printf("\n======================== BAO CAO SMART GATEWAY ========================\n");
    printf("Thiet bi          : [%d] %s\n", device.deviceID, device.deviceName);
    printf("Cong suat dinh muc: %.2f kW\n", device.ratedPowerKW);
    printf("Cam bien          : %.1f V | %.2f A | Vang nguoi %d phut\n",
           sensor.voltageVolts, sensor.currentAmperes, sensor.unoccupancyMinutes);
    printf("-----------------------------------------------------------------------\n");

    if (canhBaoQuaTai) {
        printf("!!! CANH BAO NGUY CO CHAY NO: Dong dien %.2f A vuot nguong an toan %.1f A.\n",
               sensor.currentAmperes, NGUONG_DONG_DIEN);
        printf("!!! Da kich hoat NGAT KHAN CAP thiet bi.\n");
    }
    if (daTuDongTat) {
        printf("[TIET KIEM NANG LUONG] Phong trong %d phut (>= %d phut) -> Da tu dong tat thiet bi.\n",
               sensor.unoccupancyMinutes, NGUONG_VANG_NGUOI);
    }
    if (!canhBaoQuaTai && !daTuDongTat) {
        printf("Khong co canh bao. Thiet bi giu nguyen trang thai.\n");
    }

    printf("Trang thai thiet bi: %s -> %s\n",
           statusBanDau ? "DANG BAT" : "DA TAT",
           device.status ? "DANG BAT" : "DA TAT");
    printf("-----------------------------------------------------------------------\n");

    if (!hoaDonHopLe) {
        printf("[TU CHOI TINH HOA DON] So gio van hanh khong hop le.\n");
        printf("Gia tri hop le: lon hon 0 va khong vuot qua %.0f gio/thang.\n", GIO_TOI_DA_THANG);
    } else {
        printf("So gio van hanh   : %.2f gio\n", operatingHours);
        printf("Dien nang tieu thu: %.2f x %.2f = %.2f kWh\n",
               device.ratedPowerKW, operatingHours, kWh);
        printf("Chi tiet bac thang EVN:\n");
        printf("  Bac 1 (0 - 50 kWh)    : %8.2f kWh x %.0f = %14.2f VND\n",
               kWhBac1, GIA_BAC_1, kWhBac1 * GIA_BAC_1);
        if (kWhBac2 > 0.0) {
            printf("  Bac 2 (51 - 100 kWh)  : %8.2f kWh x %.0f = %14.2f VND\n",
                   kWhBac2, GIA_BAC_2, kWhBac2 * GIA_BAC_2);
        }
        if (kWhBac3 > 0.0) {
            printf("  Bac 3 (101 - 200 kWh) : %8.2f kWh x %.0f = %14.2f VND\n",
                   kWhBac3, GIA_BAC_3, kWhBac3 * GIA_BAC_3);
        }
        if (kWhBac4 > 0.0) {
            printf("  Bac 4 (tu 201 kWh)    : %8.2f kWh x %.0f = %14.2f VND\n",
                   kWhBac4, GIA_BAC_4, kWhBac4 * GIA_BAC_4);
        }

        /* Lam tron den dong va them dau cham hang nghin */
        soTien = (long long)(tienDien + 0.5);
        sprintf(chuoiSo, "%lld", soTien);
        doDai = (int)strlen(chuoiSo);
        k = 0;
        for (i = 0; i < doDai; i++) {
            chuoiTien[k++] = chuoiSo[i];
            if ((doDai - i - 1) % 3 == 0 && i != doDai - 1) {
                chuoiTien[k++] = '.';
            }
        }
        chuoiTien[k] = '\0';
        printf("TONG TIEN DIEN (chua VAT): %s VND\n", chuoiTien);
    }
    printf("=======================================================================\n");

    return 0;
}
