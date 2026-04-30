#!/bin/bash

# Tetrimino Space Invaders - GitHub Repository Setup Script
# This script helps you initialize and push to GitHub

echo "=================================="
echo "Tetrimino Space Invaders"
echo "GitHub Repository Setup"
echo "=================================="
echo ""

# Check if git is installed
if ! command -v git &> /dev/null; then
    echo "❌ Error: git is not installed"
    echo "Install git first: sudo apt install git"
    exit 1
fi

echo "✅ Git is installed"
echo ""

# Get GitHub username and repo name
read -p "Enter your GitHub username: " GITHUB_USER
read -p "Enter repository name [Tetriminos]: " REPO_NAME
REPO_NAME=${REPO_NAME:-Tetriminos}

echo ""
echo "Repository URL will be:"
echo "https://github.com/$GITHUB_USER/$REPO_NAME.git"
echo ""
read -p "Is this correct? (y/n): " CONFIRM

if [[ $CONFIRM != "y" && $CONFIRM != "Y" ]]; then
    echo "Setup cancelled"
    exit 0
fi

echo ""
echo "Setting up repository..."

# Initialize git if not already done
if [ ! -d .git ]; then
    git init
    echo "✅ Initialized git repository"
else
    echo "ℹ️  Git repository already initialized"
fi

# Add all files
git add .
echo "✅ Added files to git"

# Initial commit
git commit -m "Initial commit - Tetrimino Space Invaders v1.1

- Classic Space Invaders gameplay with Tetris theme
- Enemy shooting system (starts Wave 2)
- Bonus lives every 1000 points
- Progressive difficulty
- Complete documentation and pre-built binaries"

echo "✅ Created initial commit"

# Set main branch
git branch -M main
echo "✅ Set main branch"

# Add remote
git remote add origin "https://github.com/$GITHUB_USER/$REPO_NAME.git"
echo "✅ Added remote origin"

echo ""
echo "=================================="
echo "Repository is ready to push!"
echo "=================================="
echo ""
echo "Next steps:"
echo ""
echo "1. Create the repository on GitHub:"
echo "   https://github.com/new"
echo "   - Name: $REPO_NAME"
echo "   - Description: Classic Space Invaders with Tetris theme for M5StickC Plus2"
echo "   - Don't initialize with README, .gitignore, or license"
echo ""
echo "2. Push to GitHub:"
echo "   git push -u origin main"
echo ""
echo "3. Add topics on GitHub:"
echo "   m5stack, m5stickc-plus2, arduino, esp32, game, space-invaders,"
echo "   tetris, retro-gaming, platformio"
echo ""
echo "4. Create release (optional):"
echo "   - Tag: v1.1.0"
echo "   - Title: Tetrimino Space Invaders v1.1 - Feature Complete"
echo "   - Upload: releases/Tetriminos-v1.1-MERGED.bin"
echo ""
echo "=================================="
