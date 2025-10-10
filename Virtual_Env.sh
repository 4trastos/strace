#!/bin/bash
# virtual environment for testing the strace program

echo "🚀 Iniciando entorno de desarrollo..."

docker run -it --rm \
  -v $(pwd):/workspace \
  -w /workspace \
  ubuntu:22.04 \
  bash -c "apt update && apt install -y build-essential gcc make vim strace && bash"
