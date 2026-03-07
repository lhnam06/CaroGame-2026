# CaroGame - Đồ Án Cờ Caro (C++)

Trò chơi Cờ Caro viết bằng **C++** (lập trình thủ tục, không OOP).
Cấu trúc tham khảo: [thng292/CaroGame](https://github.com/thng292/CaroGame)

## Biên dịch

```bash
g++ -I. -o Caro.exe main.cpp Defs/Defs.cpp View/View.cpp Model/Model.cpp Control/Control.cpp SaveLoad/SaveLoad.cpp Menu/Menu.cpp
```

Hoặc dùng Makefile:
```bash
make
```

## Chạy
```bash
./Caro.exe
```

## Điều khiển
- **W, A, S, D** – Di chuyển
- **Enter** – Đánh dấu X/O
- **L** – Lưu game
- **T** – Tải game
- **ESC** – Thoát

## Cấu trúc thư mục
```
CaroGame/
├── main.cpp
├── Defs/      – Hằng số, struct, biến toàn cục
├── View/      – Giao diện màn hình
├── Model/     – Dữ liệu và logic bàn cờ
├── Control/   – Điều khiển di chuyển, Start/Exit
├── SaveLoad/  – Lưu/Tải game
├── Menu/      – Menu chính
├── UI/        – Khung UI game (giao diện nâng cao)
├── Audio/     – Khung âm thanh (sound effects, nhạc nền)
└── ROADMAP.md – Kế hoạch mở rộng UI + Audio
```

Chi tiết các bước phát triển UI và Audio xem trong `ROADMAP.md` và `UI/README.md`, `Audio/README.md`.
