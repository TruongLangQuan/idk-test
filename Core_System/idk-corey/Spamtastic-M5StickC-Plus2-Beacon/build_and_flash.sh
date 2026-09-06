#!/bin/bash
# SPAMTASTIC! Build and Flash Script

echo "🚀 Building SPAMTASTIC Beacon Master..."

# Build the project
pio run

if [ $? -eq 0 ]; then
    echo "✅ Build successful!"
    echo "📦 Ready to flash - use 'pio run --target upload' to flash to device"
    echo "🔥 Or use the pre-built M5Burner package in releases/v1.0-Final/"
else
    echo "❌ Build failed!"
    exit 1
fi