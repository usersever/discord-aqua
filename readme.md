# Discord Bot

## Giới thiệu

Bot Discord này được viết bằng ngôn ngữ lập trình C++. Bot sử dụng nhiều thư viện để cung cấp các tính năng và kết nối cần thiết cho việc hoạt động.

## Các yêu cầu

- **Ngôn ngữ lập trình**: C++
- **Thư viện**:
  - `fmt`: Thư viện định dạng chuỗi hiện đại.
  - `dpp`: Thư viện C++ để phát triển bot Discord.
  - `curl`: Công cụ và thư viện để truyền dữ liệu với URL cú pháp.
  - `openssl`: Thư viện mạnh mẽ cung cấp các giao thức bảo mật và mật mã.
  - `libmysql`: Thư viện để kết nối và thao tác với cơ sở dữ liệu MySQL.

## Hướng dẫn Build

### Trên Linux

Để build bot trên Linux, bạn cần cài đặt các thư viện yêu cầu và sử dụng `cmake` để tạo project. Các bước cụ thể như sau:

1. **Cài đặt các thư viện yêu cầu**:
    ```sh
    sudo apt-get update
    sudo apt-get install -y libfmt-dev libcurl4-openssl-dev libssl-dev libmysqlclient-dev cmake g++
    ```

2. **Clone repository và chuyển vào thư mục**:
    ```sh
    git clone https://github.com/usersever/discord-aqua.git
    cd discord-bot
    ```

3. **Tạo thư mục build và chạy `cmake`**:
    ```sh
    mkdir build
    cd build
    cmake ..
    make
    ```

4. **Chạy bot**:
    ```sh
    ./discord-bot
    ```

### Trên Windows

Để build bot trên Windows, bạn cần cài đặt Visual Studio và các thư viện yêu cầu. Các bước cụ thể như sau:

1. **Cài đặt Visual Studio**:
    - Tải và cài đặt Visual Studio từ [đây](https://visualstudio.microsoft.com/).
    - Chọn cài đặt `Desktop development with C++`.

2. **Cài đặt các thư viện yêu cầu**:
    - Tải và cài đặt các thư viện yêu cầu bằng cách sử dụng [vcpkg](https://github.com/microsoft/vcpkg).

    ```sh
    git clone https://github.com/Microsoft/vcpkg.git
    cd vcpkg
    .\bootstrap-vcpkg.bat
    .\vcpkg integrate install
    .\vcpkg install fmt curl openssl libmysql
    ```

3. **Clone repository và mở dự án bằng Visual Studio**:
    ```sh
    git clone https://github.com/username/discord-bot.git
    cd discord-bot
    ```

4. **Mở project Visual Studio**:
    - Mở Visual Studio và chọn `File` > `Open` > `Project/Solution`.
    - Chọn tệp `mybot.vcxproj` trong thư mục dự án của bạn.

5. **Build và chạy bot**:
    - Nhấn `Ctrl+Shift+B` để build dự án.
    - Nhấn `Ctrl+F5` để chạy bot.

## Đóng góp

Nếu bạn muốn đóng góp cho dự án này, vui lòng mở Pull Request hoặc tạo Issue trên GitHub.

## Giấy phép

Dự án này được phân phối dưới giấy phép Apache 2.0. Xem tệp `LICENSE` để biết thêm chi tiết.
