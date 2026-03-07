# CaroGame - Roadmap & Cấu trúc mở rộng

Tài liệu mô tả cấu trúc thư mục và kế hoạch phát triển UI + Audio.

---

## Cấu trúc tổng quan

```
CaroGame/
├── main.cpp
├── Defs/           ✅ Đã có – Hằng số, struct, biến toàn cục
├── View/           ✅ Đã có – Giao diện cơ bản (console)
├── Model/          ✅ Đã có – Logic bàn cờ
├── Control/        ✅ Đã có – Điều khiển game
├── SaveLoad/       ✅ Đã có – Lưu/Tải
├── Menu/           ✅ Đã có – Menu đơn giản
│
├── UI/             📋 Khung – Giao diện game nâng cao
│   └── README.md
│
└── Audio/          📋 Khung – Âm thanh & hiệu ứng
    └── README.md
```

---

## UI – Giao diện game

**Mục tiêu:** Giao diện rõ ràng, dễ dùng, gần với các game console thực tế.

| Thành phần  | Mô tả ngắn |
|-------------|------------|
| Screens     | Màn hình riêng: Main, Game, Settings, Pause, About |
| Components  | Button, khung, ô nhập, thanh trạng thái... |
| Theme       | Màu, font, ký tự vẽ khung |
| Assets      | Logo ASCII, bảng điểm, icon text |
| Layout      | Tọa độ, kích thước từng vùng hiển thị |

---

## Audio – Âm thanh

**Mục tiêu:** Hiệu ứng âm thanh và nhạc nền để game sinh động hơn.

| Thành phần  | Mô tả ngắn |
|-------------|------------|
| Effects     | Click menu, đánh cờ, thắng, thua, hòa |
| Music       | Nhạc nền menu, nhạc khi chơi |
| Player      | Load, play, pause, volume |
| Resources   | File .wav, .mp3 |

---

## Thứ tự triển khai đề xuất

1. **UI:** Bắt đầu với theme (màu, ký tự vẽ) và components cơ bản.
2. **UI:** Thiết kế từng màn hình, chỉnh layout.
3. **Audio:** Chọn thư viện, chuẩn bị file âm thanh.
4. **Audio:** Viết module phát nhạc/effect.
5. **Tích hợp:** Gắn UI mới và Audio vào luồng game hiện tại.

---

*Chỉ là khung thư mục và kế hoạch – chưa có code triển khai.*
