# Game Khủng Long Chrome

Tái tạo trò chơi khủng long khi mất mạng của Google Chrome, chạy trên vi điều khiển STM32 NUCLEO-F401RE kết hợp với Kit mở rộng LUMI.

## Tính Năng

* Màn hình menu với con trỏ điều hướng
* Hai mức tốc độ (Chậm / Nhanh)
* Theo dõi điểm cao nhất giữa các lượt chơi
* Âm thanh buzzer khi nhảy
* Đèn LED đỏ nhấp nháy khi thua
* Màn hình thua hiển thị điểm trong 2 giây, sau đó tự động quay về menu

## Phần Cứng

|Thành phần|Mô tả|
|-|-|
|**Vi điều khiển**|STM32 NUCLEO-F401RE|
|**Kit mở rộng**|LUMI IoT STM Board Kit (LM-ISBK V1.0)|
|**Màn hình**|ST7735 128x128 SPI LCD (tích hợp trên kit)|
|**Buzzer**|PC9 (điều khiển qua transistor)|
|**LED**|LED RGB (điều khiển qua transistor)|
|**Nút bấm**|SW1 (PB5), SW3 (PA4), SW5 (PB4)|

## Điều Khiển

|Nút|Trong Menu|Trong Game|
|-|-|-|
|B1 (SW1, PB5)|Di chuyển con trỏ lên|—|
|B5 (SW5, PB4)|Di chuyển con trỏ xuống|—|
|B3 (SW3, PA4)|Chọn / xác nhận|Nhảy|

## Lưu Đồ Thuật Toán

### Tổng quan hoạt động chương trình

```mermaid

flowchart TD

&#x20;   A\[Khởi động hệ thống] --> B\[Cấu hình Clock 84MHz]

&#x20;   B --> C\[Khởi tạo ngoại vi: Timer, LED, Buzzer, Button, LCD]

&#x20;   C --> D\[Hiển thị Menu]

&#x20;   D --> E{Trạng thái?}

&#x20;

&#x20;   E -->|ST\_MENU| F{Nút được nhấn?}

&#x20;   F -->|B1| G\[Di chuyển con trỏ lên]

&#x20;   F -->|B5| H\[Di chuyển con trỏ xuống]

&#x20;   F -->|B3 + Start Game| I\[Bắt đầu game]

&#x20;   F -->|B3 + Speed| J\[Đổi tốc độ Chậm/Nhanh]

&#x20;   G --> E

&#x20;   H --> E

&#x20;   J --> E

&#x20;

&#x20;   I --> K\[Xóa màn hình, vẽ mặt đất và khủng long]

&#x20;   K --> L{Trạng thái ST\_PLAY}

&#x20;

&#x20;   E -->|ST\_PLAY| L

&#x20;   L --> M{B3 được nhấn?}

&#x20;   M -->|Có| N\[Khủng long nhảy + Buzzer kêu]

&#x20;   M -->|Không| O\[Chờ frame tiếp theo]

&#x20;   N --> O

&#x20;

&#x20;   O --> P\[Cập nhật vị trí khủng long]

&#x20;   P --> Q\[Cập nhật vị trí xương rồng]

&#x20;   Q --> R{Va chạm?}

&#x20;

&#x20;   R -->|Không| S\[Vẽ khủng long và xương rồng]

&#x20;   S --> T\[Cập nhật điểm số]

&#x20;   T --> L

&#x20;

&#x20;   R -->|Có| U\[GAME OVER]

&#x20;   U --> V\[Bật LED đỏ + Buzzer]

&#x20;   V --> W\[Hiển thị điểm và điểm cao nhất]

&#x20;   W --> X\[Chờ 2 giây]

&#x20;

&#x20;   E -->|ST\_OVER| X

&#x20;   X --> Y{Hết 2 giây?}

&#x20;   Y -->|Chưa| X

&#x20;   Y -->|Rồi| Z\[Tắt LED + Buzzer]

&#x20;   Z --> D

```

### Chi tiết xử lý nhảy

```mermaid

flowchart TD

&#x20;   A\[B3 được nhấn + đang trên mặt đất] --> B\[jumping = 1, jumpFr = 0]

&#x20;   B --> C\[Bật Buzzer 80ms]

&#x20;   C --> D{jumpFr <= 7?}

&#x20;   D -->|Có| E\[Khủng long bay lên: dinoY giảm]

&#x20;   D -->|Không| F{jumpFr <= 14?}

&#x20;   F -->|Có| G\[Khủng long rơi xuống: dinoY tăng]

&#x20;   F -->|Không| H\[Hạ cánh: dinoY = mặt đất, jumping = 0]

&#x20;   E --> I\[jumpFr++]

&#x20;   G --> I

&#x20;   I --> D

```

### Chi tiết phát hiện va chạm

```mermaid

flowchart TD

&#x20;   A\[Kiểm tra va chạm AABB] --> B{DINO\_X < cactX + CACT\_W?}

&#x20;   B -->|Không| C\[Không va chạm]

&#x20;   B -->|Có| D{DINO\_X + DINO\_W > cactX?}

&#x20;   D -->|Không| C

&#x20;   D -->|Có| E{dinoY < CACT\_GY + CACT\_H?}

&#x20;   E -->|Không| C

&#x20;   E -->|Có| F{dinoY + DINO\_H > CACT\_GY?}

&#x20;   F -->|Không| C

&#x20;   F -->|Có| G\[VA CHẠM → Game Over]

```

## Thư Viện Yêu Cầu

Dự án sử dụng thư viện LUMI SDK cho kit mở rộng NUCLEO-F401RE:

**https://github.com/HD-Nam/ThuVien\_SDK\_1.0.3\_NUCLEO-F401RE**

Clone và đặt cùng thư mục với dự án:

```
D:/STM32CubeIDE\\\_2.1.1/
├── ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/   ← Thư viện SDK
│   ├── lib\\\_stm/
│   └── shared/
└── DinoGame/                          ← Dự án này
    ├── Src/
    │   └── main.c
    ├── Inc/
    ├── Startup/
    ├── STM32F401RETX\\\_FLASH.ld
    └── STM32F401RETX\\\_RAM.ld
```

## Hướng Dẫn Build

### Yêu cầu

* [STM32CubeIDE](https://www.st.com/en/development-tools/stm32cubeide.html)
* [LUMI SDK](https://github.com/HD-Nam/ThuVien_SDK_1.0.3_NUCLEO-F401RE) đã clone về máy

### Các bước thực hiện

1. **Clone repo và SDK:**

```bash
   git clone https://github.com/HD-Nam/ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE.git
   git clone <đường-dẫn-repo-này>
   ```

2. **Mở STM32CubeIDE**, import dự án qua **File > Import > Existing Projects into Workspace**
3. **Cấu hình đường dẫn Include** trong **Project Properties > C/C++ Build > Settings > MCU GCC Compiler > Include paths**:

```
   ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/shared/Drivers/CMSIS/Include
   ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/shared/Drivers/STM32F401RE\\\_StdPeriph\\\_Driver/inc
   ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/shared/Middle/button
   ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/shared/Middle/buzzer
   ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/shared/Middle/led
   ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/shared/Middle/sensor
   ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/shared/Middle/serial
   ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/shared/Middle/ucglib
   ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/shared/Middle/flash
   ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/shared/Middle/rtos
   ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/shared/Utilities
   ```

4. **Cấu hình đường dẫn thư viện** trong **MCU GCC Linker > Library search paths**:

```
   ThuVien\\\_SDK\\\_1.0.3\\\_NUCLEO-F401RE/lib\\\_stm
   ```

5. **Build** bằng **Ctrl+B**
6. **Nạp chương trình** qua **Run > Debug** hoặc sử dụng ST-LINK programmer

## Cấu Trúc Dự Án

```
├── Src/
│   └── main.c                    ← Mã nguồn game
├── Inc/                          ← Header của dự án (nếu có)
├── Startup/
│   └── startup\\\_stm32f401retx.s   ← File khởi động
├── STM32F401RETX\\\_FLASH.ld        ← Linker script
├── STM32F401RETX\\\_RAM.ld
├── .gitignore
└── README.md
```

## Cách Hoạt Động

### Tổng quan

Người chơi có thể chọn độ khó trên màn hình menu khi mở game. Sau khi bắt đầu, người chơi điều khiển khủng long với mục tiêu nhảy qua càng nhiều xương rồng càng tốt. Game sẽ theo dõi điểm số và hiển thị trên menu. Khi thua, người chơi sẽ được hiển thị điểm hiện tại và điểm cao nhất, sau đó quay về menu.

### Chi tiết kỹ thuật

* **Tốc độ khung hình:** Cố định 60ms/frame (khoảng 16 FPS) cho mọi mức tốc độ
* **Tốc độ xương rồng:** Chậm = 6 pixel/frame, Nhanh = 9 pixel/frame
* **Nhảy:** 14 frame (7 frame bay lên + 7 frame rơi xuống), độ cao 35 pixel
* **Va chạm:** Sử dụng thuật toán AABB (Axis-Aligned Bounding Box)
* **3 loại xương rồng:** Xoay vòng ngẫu nhiên mỗi lần xuất hiện lại

### Sơ đồ trạng thái

```
┌──────────┐    B3+Start    ┌──────────┐   Va chạm   ┌──────────┐
│  ST\\\_MENU │ ──────────────>│ ST\\\_PLAY  │ ──────────> │ ST\\\_OVER  │
│          │                │          │             │          │
│ - Chọn   │                │ - Nhảy   │             │ - LED đỏ │
│   tốc độ │                │ - Điểm   │             │ - Buzzer │
│ - Điểm   │                │ - Buzzer │             │ - 2 giây │
│   cao    │                │          │             │          │
└──────────┘ <──────────────└──────────┘             └──────────┘
       ^                                                   │
       └───────────────── Sau 2 giây ──────────────────────┘
```

## Giấy Phép

MIT

