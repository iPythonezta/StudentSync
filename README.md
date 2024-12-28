# Student Management System

## Table of Contents
1. [Project Description](#project-description)
2. [Prerequisites](#prerequisites)
3. [Installation Guide](#installation-guide)
    - [Download Git](#download-git)
    - [Download Vcpkg](#download-vcpkg)
    - [Download Required Libraries](#download-required-libraries)
4. [Cloning the Repository](#cloning-the-repository)
5. [Running the Program](#running-the-program)

---

## Project Description
The program is designed to support student synchronization and academic management workflows for institutions. This program implements a student management system with a web API using the Crow framework, SQLite database, and JWT authentication.

### Key Functionalities:
- **User Authentication**: Create accounts, login, password management, and role-based authorization with admin-user distinction.
- **Event and Task Management**: Create, retrieve, and delete scheduled events and tasks.
- **Subject Management**: Manage subject marks and calculate aggregates using set weightages for quizzes, assignments, and exams.
- **Marks Management**: Update quizzes, assignments, mids, and final exams' marks.
- **Group Formation**: Automatically form student groups based on preferences.
- **API Endpoints**: Provides a RESTful interface for interacting with the system.
- **File Handling**: Stores and retrieves questions and answers, student marks, and preferences in CSV files.
- **JWT Integration**: Secure token-based user authentication and authorization.

---

## Prerequisites
- Visual Studio (or any C++ IDE)
- Git
- Vcpkg (C++ package manager)
- Libraries: Crow, SQLite3, JWT-CPP, Regex

---

## Installation Guide

### Download Git
1. Go to the official Git website: [https://git-scm.com/](https://git-scm.com/).
2. Download and install the correct version for your operating system accordint to the instructions there.
3. Complete the installation and verify by running the following command:

   ```bash
   git --version
   ```

---

### Download Vcpkg
1. Navigate to the folder where you want to install Vcpkg.
2. Open a terminal in that folder and run the following commands:

   ```bash
   git clone https://github.com/microsoft/vcpkg.git
   cd vcpkg
   .\bootstrap-vcpkg.bat
   .\vcpkg integrate install
   ```

3. To use Vcpkg globally from any directory, add its full path to your system's `PATH` environment variable.

---

### Download Required Libraries
To install all required libraries, run:

```bash
.\vcpkg install regex crow sqlite3 jwt-cpp
```

If you encounter issues, install each library individually:

```bash
.\vcpkg install crow
.\vcpkg install sqlite3
.\vcpkg install jwt-cpp
.\vcpkg install regex
```

---

## Cloning the Repository
1. Open Visual Studio and create a new project. Name the solution file as desired.
2. Open the terminal from the `View` tab in Visual Studio.
3. Run the following commands one-by-one:

   ```bash
   cd your/project/folder/
   git remote add origin <repository-url>
   git fetch origin
   git checkout -f main
   ```

4. Add the `main.cpp` file to the `Source Files` section in Solution Explorer.

---

## Running the Program
1. Build and run the project in Visual Studio.
2. A link to the locally hosted web app will be displayed (e.g., `http://0.0.0.0:PORT`).
3. Press `Ctrl+Click` on the link to open it in your browser.
4. Replace `0.0.0.0` in the URL with `localhost` and press Enter.
5. Optionally, port-forward to a static domain using a free hosting service for wider access.
