# Use an official Debian-based image with SDL2 support
FROM debian:latest

# Install necessary packages
RUN apt-get update && apt-get install -y \
    libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev libsdl2-mixer-dev \
    gcc make build-essential \
    libgl1-mesa-glx libgl1-mesa-dri \
    tcc \
    && rm -rf /var/lib/apt/lists/*

# Set the working directory
WORKDIR /flappy-bird-main

# Copy game source files
COPY . .
COPY ./res /flappy-bird-main/res


# Compile the game
RUN ./run-linux.sh

# Run the game
CMD ["./game"]
