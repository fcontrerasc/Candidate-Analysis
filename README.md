# Candidate Analysis

This project is a Candidate Analysis application that parses JSON and XML files to extract candidate information and stores it in a database.

## Prerequisites

- Visual Studio 2019 or later
- vcpkg (Visual Studio package manager)

## Setup Instructions

1. **Clone the Repository**

```bash
git clone https://github.com/fcontrerasc/Candidate-Analysis.git
```

2. **Open the Project in Visual Studio**

   - Open Visual Studio.
   - Go to `File` > `Open` > `Project/Solution`.
   - Select the `Candidate-Analysis.sln` file from the cloned repository.

3. **Install vcpkg**

   If you don't have vcpkg installed, you can install it with the Visual Studio Installer.

4. **Integrate vcpkg with Visual Studio**

   Run the following command inside Visual Studio terminal to integrate vcpkg:
   
```bash
.\vcpkg integrate install
```

   **IMPORTANT** Make sure Visual Studio is using the manifest vcpkg.json file to recognize the dependencies.

6. **Build and Run the Application**

   - In Visual Studio, select the build configuration (e.g., Debug or Release).
   - Build the solution by going to `Build` > `Build Solution`.
   - Run the application by going to `Debug` > `Start Without Debugging`.

## Project Structure

- `DataDownloader.cpp` / `DataDownloader.h`: Handles downloading data from URLs.
- `DataParser.cpp` / `DataParser.h`: Parses JSON and XML files to extract candidate information.
- `DatabaseManager.cpp` / `DatabaseManager.h`: Manages the SQLite database for storing candidate information.
- `DataAnalyzer.cpp` / `DataAnalyzer.h`: Analyzes the candidate data.
- `Logger.cpp` / `Logger.h`: Logs messages and errors.
- `Candidate-Analysis.cpp`: Entry point of the application.