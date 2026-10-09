#include <stdio.h>

// Khai bao cau truc quan ly thiet bi thong minh
struct SmartDevice {
    int id;
    float voltage;      // Dien ap (V)
    float current;      // Dong dien (A)
    float power_watt;   // Cong suat tieu thu (W)
    int is_active;      // Trang thai thiet bi: 1 - Bat, 0 - Tat
    int is_overloaded;  // Canh bao qua tai: 1 - Qua tai, 0 - An toan
};

int main() {
    // Khoi tao thong tin thiet bi Binh Nong Lanh
    struct SmartDevice heater;
    heater.id = 101;
    heater.voltage = 220.0f;
    heater.current = 35.0f;     // Cam bien ghi nhan dong dien 35A (vuot nguong 30A)
    heater.is_active = 1;       // Nguoi dung bam BAT thiet bi
    heater.is_overloaded = 0;   // Mac dinh ban dau an toan
    heater.power_watt = 0.0f;   // Khoi tao cong suat, tranh gia tri rac

    // BUOC 1: Kiem tra quy tac an toan qua tai TRUOC khi tinh cong suat.
    // Chi xet khi thiet bi dang BAT; vuot 30A thi bat canh bao va tu dong ngat dien.
    if (heater.is_active == 1 && heater.current > 30.0f) {
        heater.is_overloaded = 1;
        heater.is_active = 0;   // Tu dong ngat dien thiet bi
    }

    // BUOC 2: Tinh cong suat dua tren trang thai CUOI CUNG cua thiet bi.
    if (heater.is_active == 1) {
        heater.power_watt = heater.voltage * heater.current;   // P = V x I
    } else {
        heater.power_watt = 0.0f;   // TAT (hoac vua bi ngat) -> bat buoc 0.0 W, bo qua dong ro/nhieu
    }

    // In thong so thiet bi ra man hinh ung dung IoT
    printf("=== HE THONG GIAM SAT THIET BI SMART HOME ===\n");
    printf("Ma thiet bi: %d\n", heater.id);
    printf("Trang thai hoat dong: %s\n", heater.is_active ? "BAT" : "TAT");
    printf("Cong suat tieu thu: %.2f W\n", heater.power_watt);
    printf("Canh bao qua tai (>30A): %s\n", heater.is_overloaded ? "CO (DA NGAT DIEN)" : "KHONG");

    return 0;
}
