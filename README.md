🧵 Multi-threaded File Downloader in C


Author: Cheran Balakrishnan

📌 Overview
This project is a multi-threaded file downloader built in C using pthreads and libcurl. It reads URLs from a file (downloads.txt by default), spawns multiple threads, and downloads each file concurrently. You can optionally specify a target directory using -o <folder>.

🚀 Features
- ✅ Downloads files from URLs listed in a .txt file
- ✅ Uses multiple threads for concurrent downloading
- ✅ Displays download progress in real-time
- ✅ Supports custom download directory with -o <folder>
- ✅ Gracefully handles missing filenames in URLs (auto-generates defaults)
- ✅ Thread-safe logging and resource cleanup
- ✅ Compatible with Linux/WSL/macOS



Demo video: https://www.youtube.com/watch?v=wcEpOWS3N7U&ab_channel=cheranB 



📂 Project Structure
.
├── downloader.c         # Source code
├── downloads.txt        # (Sample) List of URLs to download
├── README.txt           # This file

🧪 Sample Run
./downloader

To save into a specific directory:
./downloader -o my_downloads

To specify a different input file:
./downloader links.txt
./downloader links.txt -o my_downloads

⚙️ Installation

Ubuntu / WSL:
sudo apt update
sudo apt install libcurl4-openssl-dev

macOS (with Homebrew):
brew install curl

🛠️ Compilation
Use gcc with -lcurl flag:
gcc downloader.c -o downloader -lcurl

📄 Input Format
The file containing URLs (default: downloads.txt) should have one URL per line. Example:

https://upload.wikimedia.org/wikipedia/commons/thumb/4/47/PNG_transparency_demonstration_1.png/640px-PNG_transparency_demonstration_1.png
https://sample-videos.com/img/Sample-jpg-image-500kb.jpg
https://sample-videos.com/img/Sample-png-image-500kb.png

🧠 Key Concepts
- Threads: Each file is downloaded in a separate thread (pthread).
- libcurl: Used for HTTP/HTTPS downloads.
- Mutex Locks: Ensures thread-safe access to shared resources.
- Progress Callback: Shows download progress using CURLOPT_XFERINFOFUNCTION.
- Dynamic File Naming: Handles URLs without filenames by generating default ones like File_1.txt.

⚠️ Notes
- Overwrites files if a file with the same name already exists.
- If directory already exists, it reuses it — if not, it creates it using mkdir.
- Fails gracefully with meaningful error messages for:
  - Invalid URLs
  - File creation issues
  - Curl initialization problems

📌 To Do / Future Work
- [ ] Retry mechanism on failed downloads
- [ ] Logging to a file
- [ ] CLI flags for thread count, timeout, etc.

📃 License
This project is open source and free to use. 
