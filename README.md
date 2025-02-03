# Flappy Bird in C (SDL2)

Flappy Bird clone written in C using the SDL2 library.

## 📌 Features
- Classic Flappy Bird mechanics 🐦
- Simple graphics using SDL2 🎮
- Sound effects and animations 🔊

## 🛠️ Requirements
Before running the game, ensure you have the following dependencies installed:

### Linux
```bash
sudo apt-get update && sudo apt-get install -y \
    libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev \
    gcc make build-essential tcc
```

### Windows (Using MinGW)
Download and install [SDL2](https://github.com/libsdl-org/SDL/releases) and place the required DLLs in the project directory.

## 🚀 Installation & Execution

### 1. Clone the repository
```bash
git clone https://github.com/yourusername/flappy-bird-c.git
cd flappy-bird-c
```

### 2. Run the game using scripts
Use the appropriate script for your operating system:

#### Linux
```bash
chmod +x run-linux.sh
./run-linux.sh
```

#### macOS
```bash
chmod +x run-mac.sh
./run-mac.sh
```

#### Windows
```batch
run-windows.bat
```

## 🐳 Running with Docker
You can also run the game inside a Docker container.

### 1. Build the Docker image
```bash
sudo docker build -t flappy-bird:latest .
```

### 2. Run the game using Docker
```bash
sudo docker run --rm -it \
  -e DISPLAY=$DISPLAY \
  -v /tmp/.X11-unix:/tmp/.X11-unix \
  --privileged \
  flappy-bird:latest
```

## 📷 Screenshots
Not yet !

## 📜 License
This project is open-source and available under the [MIT License](LICENSE).

---
Made with ❤️ by [Your Name](https://github.com/yourusername)

