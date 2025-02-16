# **File System Manager** 📂  

## 📌 **Project Description**  
This project implements a **hierarchical file system manager** in **C**, designed to manage **directories and files** with a simple API. It provides **directory traversal, file creation, deletion, and memory tracking**, operating within a **predefined virtual disk space**.

---

## **🔧 Core Features**  

✔️ **Hierarchical Directory Structure** – Supports nested directories  
✔️ **File & Directory Creation** – Enables structured storage  
✔️ **Efficient Memory Tracking** – Keeps track of used and free space  
✔️ **Recursive Deletion** – Removes directories and files efficiently  
✔️ **Path Parsing & Navigation** – Supports absolute and relative paths  
✔️ **Directory Listing** – Lists files and directories with sorting options  

---

## **📜 Function Overview**  

| **Function**                         | **Description** |
|---------------------------------------|----------------|
| `my_create(int disk_size)`            | Initializes the file system with a given disk size |
| `my_destroy()`                         | Frees all allocated memory and resets the system |
| `my_create_dir(const char* path)`      | Creates a directory at the specified path |
| `my_create_file(const char* path, int file_size)` | Creates a file with a specified size |
| `my_remove(const char* path, int recursive)` | Deletes a file or directory (recursively if needed) |
| `my_change_dir(const char* path)`      | Changes the current working directory |
| `my_get_cur_dir(char* dst)`            | Retrieves the absolute path of the current directory |
| `my_list(const char* path, int dir_first)` | Lists contents of a directory (directories first if enabled) |

---

## **🚀 How It Works**  

1️⃣ **Initialization (`my_create`)**  
   - Sets up the **root directory** (`/`) as the base of the file system.  
   - Allocates memory for tracking files and directories.  

2️⃣ **File & Directory Management (`my_create_file`, `my_create_dir`)**  
   - Creates directories and files with unique names.  
   - Prevents duplicate directory names and incorrect file placements.  

3️⃣ **Navigation (`my_change_dir`, `my_get_cur_dir`)**  
   - Supports **absolute** (`/home/user/docs`) and **relative** (`../folder`) paths.  
   - Maintains a **current working directory** for easy file system traversal.  

4️⃣ **Deletion (`my_remove`)**  
   - Deletes files **individually** or **recursively** for directories.  
   - Updates the free memory count upon deletion.  

5️⃣ **Listing Contents (`my_list`)**  
   - Prints all **files and directories** in a given path.  
   - Supports **sorting** with directories appearing first.  
