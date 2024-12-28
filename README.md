`


Project Description:

    The program is designed to support student synchronization and academic management workflows for institutions. This program implements a student management system with a web API using the Crow framework, SQLite database, and JWT authentication.
    Key functionalities include:
    
    - User Authentication: Create accounts, login, password management, and role-based authorization with admin-user distinction.
    - Event and task Management: Create, retrieve, and delete scheduled events and tasks.
    - Subject Management: Manage subjects marks and calculate aggregates using set weightages for quizzes, assignments, and exams.
    - Marks Management: update quizzes, assignments, mids, and final exams' marks.
    - Group Formation: Automatically form student groups based on preferences.
    - API Endpoints: Provides a RESTful interface for interacting with the system.
    - File Handling: Stores and retrieves questions and answers, student marks and preferences in CSV files.
    - JWT Integration: Secure token-based user authentication and authorization.

.........................................................................................................................................................................................................................................................

Download Git:

Go to the official Git website: https://git-scm.com/. Click on the Download button, which should automatically detect your operating system and offer the correct version for Windows. Locate the downloaded .exe file (e.g., Git-<version>-64-bit.exe) and double-click it to start the installation. During installation, you'll encounter several configuration options. Here's a recommended setup:

-License Agreement: Click Next after accepting the GNU General Public License.

-Select Destination: Choose the default directory (e.g., C:\Program Files\Git) or a custom location. Click Next.

-Select Components: Leave the default options checked (recommended). Click Next.

-Default Editor for Git: Select a text editor for Git (e.g., Vim, Notepad++, or Visual Studio Code). Click Next.

-Adjust Environment Path: Select "Git from the command line and also from 3rd-party software" to add Git to your system’s PATH. Click Next.

-Choose HTTPS Transport Backend: Leave the default option (Use the OpenSSL library). Click Next.

-Configure Line Ending Conversions: Choose "Checkout Windows-style, commit Unix-style line endings" (recommended for cross-platform development). Click Next.

-Terminal Emulator: Select "Use MinTTY" (recommended for the Git Bash terminal). Click Next.

-Default Behavior for Pull: Leave the default option (Default (fast-forward or merge)). Click Next.

-Extra Options: Leave the additional settings as default (recommended). Click Next.

-Complete Installation Click Install and wait for the installation to complete. When prompted, check "Launch Git Bash" to open Git immediately or finish the installation.

-Verify Installation: Open the Command Prompt or Git Bash. Type the following command to check the installed version:

......................................

git --version

......................................

You should see the Git version displayed.

.........................................................................................................................................................................................................................................................

Download Vcpkg:

vcpkg is a free C/C++ package manager for acquiring and managing libraries. 

-Navigate to the folder where you want to install Vcpkg. Open File Explorer, go to the folder, right-click, and select Open in Terminal (or Open PowerShell window here).

-Run the following command in PowerShell to clone the Vcpkg repository: git clone https://github.com/microsoft/vcpkg.git

-Change the directory to the newly cloned vcpkg folder: cd vcpkg

-Run the bootstrap script to build the Vcpkg executable: .\bootstrap-vcpkg.bat

-To make Vcpkg available globally (e.g., for all C++ projects), integrate it into your system using the following command: .\vcpkg integrate install

To use Vcpkg globally from any directory in the command line, note the full path of the vcpkg folder (e.g., C:\path\to\vcpkg). Add it to your system’s PATH environment variable.


.........................................................................................................................................................................................................................................................

Download All libraries:

Using one singular command in the same powershell: .\vcpkg install regex crow sqlite3 jwt-cpp

If you are having any issues with downloading anything, then you may install each one separately by using these commands one by one:

.\vcpkg install crow

.\vcpkg install sqlite3

.\vcpkg install jwt-cpp

.\vcpkg install regex

.........................................................................................................................................................................................................................................................

Cloning the repository:

Open visula Studio and create a new project, name the solution file whatever you want

Now go to the view tab and open the terminal

The develloper powershell will open and then write the following commands

cd your/project/folder/

git remote add origin this/repository's/url (You can copy from above)

git fetch origin

git checkout -f main

Now go to your Solution explorer and add the main.cpp file in your source files. Build the file and run it. This will host the program which will give a link to the web app hosted at a locally hosted web page starting with 0.0.0.0. Ctrl+click on that link and it will direct you to a website. replace the 0.0.0.0 in the URL by localhost and then press enter. If you want, this can also be port-forwarded onto a static domain, using a free hosting service.
