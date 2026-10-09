/*
 * SMARTHOME VIETNAM - IoT Gateway: Toi uu bo nho struct & phan tich an toan thiet bi
 * Chuan C11. Bien dich: gcc -std=c11 -Wall -o main main.c
 *
 * Pham vi: chi dung struct, toan tu '.', kieu co ban, if/else, vong lap trong main().
 * Khong mang struct, khong ham tu dinh nghia.
 */
#include <stdio.h>

/* =====================================================================
 * STRUCT CHUA TOI UU (khai bao tu phat - thu tu truong lon xon)
 * Tren GCC x86-64 / ARM Cortex-M (double can le 8 byte): sizeof = 72 byte
 * ===================================================================== */
struct DeviceUnoptimized {
    unsigned char  status;          /* 1  (+7 padding)  */
    double         voltage;         /* 8                */
    unsigned char  deviceType;      /* 1  (+3 padding)  */
    float          current;         /* 4                */
    unsigned char  isOverloaded;    /* 1  (+7 padding)  */
    double         measuredPower;   /* 8                */
    unsigned short deviceID;        /* 2  (+2 padding)  */
    int            operatingMinutes;/* 4                */
    unsigned char  isLeaking;       /* 1  (+7 padding)  */
    double         energyKWh;       /* 8                */
    unsigned char  isPowerMismatch; /* 1  (+7 padding cuoi) */
};

/* =====================================================================
 * GIAI PHAP 1: STRUCT PHANG - sap xep kich thuoc giam dan  (DUOC CHON)
 * sizeof = 40 byte (39 byte du lieu + 1 byte padding cuoi)
 * ===================================================================== */
struct DeviceOptimized {
    /* --- 8 byte --- */
    double         voltage;         /* Dien ap U (V) */
    double         measuredPower;   /* Cong suat do tu thiet bi P (W) */
    double         energyKWh;       /* Dien nang tieu thu (kWh) - ket qua */
    /* --- 4 byte --- */
    float          current;         /* Dong dien I (A) */
    int            operatingMinutes;/* Thoi gian hoat dong t (phut) */
    /* --- 2 byte --- */
    unsigned short deviceID;        /* Ma thiet bi (1 - 65535) */
    /* --- 1 byte --- */
    unsigned char  deviceType;      /* 1: Binh nong lanh, 2: Dieu hoa, 3: Bep tu */
    unsigned char  status;          /* 0: Tat, 1: Bat */
    unsigned char  isOverloaded;    /* Co canh bao qua tai */
    unsigned char  isLeaking;       /* Co canh bao ro ri */
    unsigned char  isPowerMismatch; /* Co canh bao cam bien cong suat sai lech */
};

/* =====================================================================
 * GIAI PHAP 2: STRUCT LONG NHAU - nhom theo chuc nang
 * sizeof = 48 byte (padding cuoi cua tung struct con bi cong don)
 * ===================================================================== */
struct ElectricalReading {          /* Nhom so lieu cam bien: 24 byte */
    double voltage;
    double measuredPower;
    float  current;                 /* (+4 padding cuoi) */
};
struct OperationInfo {              /* Nhom thong tin van hanh: 16 byte */
    double         energyKWh;
    int            operatingMinutes;
    unsigned short deviceID;
    unsigned char  deviceType;
    unsigned char  status;
};
struct SafetyFlags {                /* Nhom co canh bao: 3 byte */
    unsigned char isOverloaded;
    unsigned char isLeaking;
    unsigned char isPowerMismatch;
};
struct DeviceNested {
    struct ElectricalReading reading;   /* 24 */
    struct OperationInfo     info;      /* 16 */
    struct SafetyFlags       flags;     /* 3 (+5 padding cuoi) */
};

int main(void) {
    /* ---------------- Hang so nghiep vu ---------------- */
    /* Nguong dong dien khai bao kieu float de so sanh CUNG do chinh xac voi dev.current
     * (neu so float 0.05f voi double 0.05 thi 0.0500000007 > 0.05 -> bao ro ri sai o bien) */
    const float  NGUONG_DONG_DIEN  = 30.0f;    /* A */
    const float  NGUONG_RO_RI      = 0.05f;    /* A */
    const double NGUONG_CONG_SUAT  = 6600.0;   /* W */
    const double SAI_LECH_TOI_DA   = 5.0;      /* % */
    const double EPSILON           = 1e-9;     /* bu sai so lam tron khi so sanh % */

    struct DeviceUnoptimized mauCu;     /* chi dung de do sizeof / tong kich thuoc truong */
    struct DeviceNested      mauLong;   /* chi dung de do sizeof / tong kich thuoc truong */
    struct DeviceOptimized   dev;       /* giai phap duoc chon - dung cho xu ly nghiep vu */

    int    tmpID, tmpType, tmpStatus, nhapOK;
    float  tmpCurrent;
    double pCalc, chenhLech, phanTramLech;
    int    duLieuLoi = 0;
    int    soCanhBao = 0;
    size_t tongDuLieu;

    /* =================== PHAN 1: SO SANH sizeof =================== */
    printf("==================== PHAN TICH BO NHO STRUCT ====================\n");

    tongDuLieu = sizeof(mauCu.status) + sizeof(mauCu.voltage) + sizeof(mauCu.deviceType)
               + sizeof(mauCu.current) + sizeof(mauCu.isOverloaded) + sizeof(mauCu.measuredPower)
               + sizeof(mauCu.deviceID) + sizeof(mauCu.operatingMinutes) + sizeof(mauCu.isLeaking)
               + sizeof(mauCu.energyKWh) + sizeof(mauCu.isPowerMismatch);
    printf("%-34s sizeof = %3zu byte | du lieu = %zu | padding = %zu\n",
           "Struct chua toi uu (tu phat)", sizeof(struct DeviceUnoptimized),
           tongDuLieu, sizeof(struct DeviceUnoptimized) - tongDuLieu);

    tongDuLieu = sizeof(dev.voltage) + sizeof(dev.measuredPower) + sizeof(dev.energyKWh)
               + sizeof(dev.current) + sizeof(dev.operatingMinutes) + sizeof(dev.deviceID)
               + sizeof(dev.deviceType) + sizeof(dev.status) + sizeof(dev.isOverloaded)
               + sizeof(dev.isLeaking) + sizeof(dev.isPowerMismatch);
    printf("%-34s sizeof = %3zu byte | du lieu = %zu | padding = %zu\n",
           "Giai phap 1: Flat (giam dan)", sizeof(struct DeviceOptimized),
           tongDuLieu, sizeof(struct DeviceOptimized) - tongDuLieu);

    tongDuLieu = sizeof(mauLong.reading.voltage) + sizeof(mauLong.reading.measuredPower)
               + sizeof(mauLong.reading.current) + sizeof(mauLong.info.energyKWh)
               + sizeof(mauLong.info.operatingMinutes) + sizeof(mauLong.info.deviceID)
               + sizeof(mauLong.info.deviceType) + sizeof(mauLong.info.status)
               + sizeof(mauLong.flags.isOverloaded) + sizeof(mauLong.flags.isLeaking)
               + sizeof(mauLong.flags.isPowerMismatch);
    printf("%-34s sizeof = %3zu byte | du lieu = %zu | padding = %zu\n",
           "Giai phap 2: Nested (long nhau)", sizeof(struct DeviceNested),
           tongDuLieu, sizeof(struct DeviceNested) - tongDuLieu);

    printf("-----------------------------------------------------------------\n");
    printf("Giai phap 1 tiet kiem %zu byte/thiet bi (%.1f%%) so voi struct cu.\n",
           sizeof(struct DeviceUnoptimized) - sizeof(struct DeviceOptimized),
           100.0 * (double)(sizeof(struct DeviceUnoptimized) - sizeof(struct DeviceOptimized))
                 / (double)sizeof(struct DeviceUnoptimized));
    printf("=> Chon GIAI PHAP 1 (Flat Struct) cho IoT Gateway.\n");

    /* =================== PHAN 2: NHAP DU LIEU =================== */
    printf("\n==================== NHAP THONG SO THIET BI ====================\n");

    printf("Ma thiet bi (1 - 65535): ");
    nhapOK = scanf("%d", &tmpID);
    while (getchar() != '\n');
    if (nhapOK != 1 || tmpID < 1 || tmpID > 65535) {
        printf("LOI: Ma thiet bi phai la so nguyen trong khoang 1 - 65535. Dung tinh toan!\n");
        return 1;
    }
    dev.deviceID = (unsigned short)tmpID;

    printf("Loai thiet bi (1: Binh nong lanh, 2: Dieu hoa, 3: Bep tu): ");
    nhapOK = scanf("%d", &tmpType);
    while (getchar() != '\n');
    if (nhapOK != 1 || tmpType < 1 || tmpType > 3) {
        printf("LOI: Loai thiet bi chi nhan 1, 2 hoac 3. Dung tinh toan!\n");
        return 1;
    }
    dev.deviceType = (unsigned char)tmpType;

    printf("Trang thai (1: Bat, 0: Tat): ");
    nhapOK = scanf("%d", &tmpStatus);
    while (getchar() != '\n');
    if (nhapOK != 1 || (tmpStatus != 0 && tmpStatus != 1)) {
        printf("LOI: Trang thai chi nhan 0 (Tat) hoac 1 (Bat). Du lieu khong hop le, dung tinh toan!\n");
        return 1;
    }
    dev.status = (unsigned char)tmpStatus;

    printf("Dien ap U (V): ");
    nhapOK = scanf("%lf", &dev.voltage);
    while (getchar() != '\n');
    if (nhapOK != 1) { dev.voltage = 0.0; duLieuLoi = 1; }

    printf("Dong dien I (A): ");
    nhapOK = scanf("%f", &tmpCurrent);
    while (getchar() != '\n');
    if (nhapOK != 1) { tmpCurrent = -1.0f; duLieuLoi = 1; }
    dev.current = tmpCurrent;

    printf("Cong suat do tu thiet bi P (W): ");
    nhapOK = scanf("%lf", &dev.measuredPower);
    while (getchar() != '\n');
    if (nhapOK != 1) { dev.measuredPower = -1.0; duLieuLoi = 1; }

    printf("Thoi gian hoat dong t (phut): ");
    nhapOK = scanf("%d", &dev.operatingMinutes);
    while (getchar() != '\n');
    if (nhapOK != 1) { dev.operatingMinutes = -1; duLieuLoi = 1; }

    /* =================== PHAN 3: KIEM TRA DU LIEU RAC =================== */
    if (duLieuLoi) {
        printf("\n[LOI] Co thong so nhap vao khong phai la so.\n");
    }
    if (dev.voltage <= 0.0) {
        printf("[LOI CAM BIEN] Dien ap U = %.2f V (yeu cau U > 0).\n", dev.voltage);
        duLieuLoi = 1;
    }
    if (dev.current < 0.0f) {
        printf("[LOI CAM BIEN] Dong dien I = %.2f A (yeu cau I >= 0).\n", dev.current);
        duLieuLoi = 1;
    }
    if (dev.measuredPower < 0.0) {
        printf("[LOI CAM BIEN] Cong suat do P = %.2f W (yeu cau P >= 0).\n", dev.measuredPower);
        duLieuLoi = 1;
    }
    if (dev.operatingMinutes < 0) {
        printf("[LOI DU LIEU] Thoi gian hoat dong t = %d phut (yeu cau t >= 0).\n",
               dev.operatingMinutes);
        duLieuLoi = 1;
    }
    if (duLieuLoi) {
        printf("=> Du lieu rac / loi cam bien. DUNG TINH TOAN!\n");
        return 1;
    }

    /* =================== PHAN 4: PHAN TICH AN TOAN =================== */
    dev.isOverloaded = 0;
    dev.isLeaking = 0;
    dev.isPowerMismatch = 0;

    /* 4.1 Cong suat tinh toan va do lech cam bien */
    pCalc = dev.voltage * (double)dev.current;
    chenhLech = dev.measuredPower - pCalc;
    if (chenhLech < 0.0) {
        chenhLech = -chenhLech;                 /* tri tuyet doi, khong can math.h */
    }
    if (pCalc > 0.0) {
        phanTramLech = chenhLech / pCalc * 100.0;
        if (phanTramLech > SAI_LECH_TOI_DA + EPSILON) {
            dev.isPowerMismatch = 1;
        }
    } else {
        /* I = 0 -> P_calc = 0: khong the chia, thiet bi bao co cong suat la sai lech */
        phanTramLech = 0.0;
        if (dev.measuredPower > 0.0) {
            dev.isPowerMismatch = 1;
        }
    }

    /* 4.2 Qua tai: I > 30 A hoac P > 6600 W (xet ca P do va P tinh de an toan) */
    if (dev.current > NGUONG_DONG_DIEN
        || dev.measuredPower > NGUONG_CONG_SUAT
        || pCalc > NGUONG_CONG_SUAT) {
        dev.isOverloaded = 1;
    }

    /* 4.3 Ro ri: thiet bi TAT nhung van co dong > 0.05 A */
    if (dev.status == 0 && dev.current > NGUONG_RO_RI) {
        dev.isLeaking = 1;
    }

    /* 4.4 Dien nang: ep kieu sang so thuc truoc khi chia de tranh chia nguyen */
    dev.energyKWh = dev.measuredPower * ((double)dev.operatingMinutes / 60.0) / 1000.0;

    /* =================== PHAN 5: XUAT BAO CAO =================== */
    printf("\n==================== BAO CAO AN TOAN THIET BI ====================\n");
    printf("Thiet bi       : #%u - %s\n", dev.deviceID,
           dev.deviceType == 1 ? "Binh nong lanh" : (dev.deviceType == 2 ? "Dieu hoa" : "Bep tu"));
    printf("Trang thai     : %s\n", dev.status ? "BAT" : "TAT");
    printf("U = %.2f V | I = %.3f A | t = %d phut\n",
           dev.voltage, dev.current, dev.operatingMinutes);
    printf("P do (thiet bi): %.2f W\n", dev.measuredPower);
    printf("P tinh (U x I) : %.2f W", pCalc);
    if (pCalc > 0.0) {
        printf("  (lech %.2f%%)\n", phanTramLech);
    } else {
        printf("\n");
    }
    printf("------------------------------------------------------------------\n");

    if (dev.isPowerMismatch) {
        printf("[CANH BAO] Cam bien cong suat bao sai lech (vuot %.0f%% so voi U x I).\n",
               SAI_LECH_TOI_DA);
        soCanhBao++;
    }
    if (dev.isOverloaded) {
        printf("[!!! CANH BAO] QUA TAI NGUY HIEM:");
        if (dev.current > NGUONG_DONG_DIEN) {
            printf(" I = %.2f A > %.1f A;", dev.current, NGUONG_DONG_DIEN);
        }
        if (dev.measuredPower > NGUONG_CONG_SUAT) {
            printf(" P do = %.2f W > %.0f W;", dev.measuredPower, NGUONG_CONG_SUAT);
        }
        if (pCalc > NGUONG_CONG_SUAT) {
            printf(" P tinh = %.2f W > %.0f W;", pCalc, NGUONG_CONG_SUAT);
        }
        printf("\n");
        soCanhBao++;
    }
    if (dev.isLeaking) {
        printf("[!!! CANH BAO] RO RI DIEN HOAC TAI AN: thiet bi TAT nhung I = %.3f A > %.2f A.\n",
               dev.current, NGUONG_RO_RI);
        soCanhBao++;
    }
    if (soCanhBao == 0) {
        printf("An toan: khong phat hien bat thuong.\n");
    }

    printf("------------------------------------------------------------------\n");
    printf("Dien nang tieu thu: %.2f W x (%d / 60.0) h / 1000 = %.4f kWh\n",
           dev.measuredPower, dev.operatingMinutes, dev.energyKWh);
    printf("==================================================================\n");

    return 0;
}
