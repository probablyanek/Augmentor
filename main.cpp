#include <opencv2/opencv.hpp>
#include <cstdlib> 
#include <iostream>
#include <windows.h>
#include <string>
#include <vector>
#include <thread>
#include <chrono>
#include <conio.h>
#include <shlobj.h>


using namespace std;
using namespace cv;

int* dimensions = new int[2];

static int range = 0;
static int progress = 0;
static int total_progress = 0;


void displayMenu(int selectedOption, int consoleWidth) {
    system("cls");  // Clears the console screen (Windows-specific)
    string title = "\x1B[36m"   // Set color to red (31 is for red)
        "\n"
        "\n"
        "\n"
        "     ___      __    __    _______ .___  ___.  _______ .__   __. .___________.  ______   .______      \n"
        "    /   \\    |  |  |  |  /  _____||   \\/   | |   ____||  \\ |  | |           | /  __  \\  |   _  \\     \n"
        "   /  ^  \\   |  |  |  | |  |  __  |  \\  /  | |  |__   |   \\|  | `---|  |----`|  |  |  | |  |_)  |    \n"
        "  /  /_\\  \\  |  |  |  | |  | |_ | |  |\\/|  | |   __|  |  . `  |     |  |     |  |  |  | |      /     \n"
        " /  _____  \\ |  `--'  | |  |__| | |  |  |  | |  |____ |  |\\   |     |  |     |  `--'  | |  |\\  \\----.\n"
        "/__/     \\__\\ \\______/   \\______| |__|  |__| |_______||__| \\__|     |__|      \\______/  | _| `._____|\n"
        "\n"
        "\n"
        "\n";

    int titleHeight = 6;  // The new ASCII art has 6 lines
    int titleWidth = 89;  // Adjusted based on the new ASCII art width

    cout << string((consoleWidth - titleWidth) / 2, ' ') << title;

    for (int i = 1; i <= 4; ++i) {  // Increased to include the "Exit" option
        if (i == selectedOption) {
            cout << "\x1B[32m"; // Set color to green (32 is for green)
            cout << "  --> ";
        }
        else {
            cout << "\x1B[0m";  // Reset color to default
            cout << "      ";
        }
        if (i == 1) {
            cout << "Classification\n"; // Display "Exit" option
        }

        else if (i == 2) {
            cout << "Detection\n"; // Display "Exit" option
        }

        else if (i == 3) {
            cout << "Exit\n"; // Display "Exit" option
        }
    }
    cout << "\x1B[0m";  // Reset color to default at the end of the loop
}


string SelectFolder() {
    BROWSEINFOA browseInfo = { 0 };
    char path[MAX_PATH];

    browseInfo.hwndOwner = NULL;
    browseInfo.pidlRoot = NULL;
    browseInfo.pszDisplayName = path;
    browseInfo.lpszTitle = "Select a Folder";
    browseInfo.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;

    LPITEMIDLIST pidl = SHBrowseForFolderA(&browseInfo);

    if (pidl != NULL) {
        if (SHGetPathFromIDListA(pidl, path)) {
            return path;
        }
    }

    return "";
}

int getUserSelectedOpt() {
    int selectedOption = 1;

    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int consoleWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;

    while (true) {
        system("cls");
        displayMenu(selectedOption, consoleWidth);

        char key = _getch();
        switch (key) {
        case 72:
            if (selectedOption > 1) {
                selectedOption--;
            }
            break;
        case 80:
            if (selectedOption < 3) {
                selectedOption++;
            }
            break;
        case 13:
            return selectedOption;
        }
    }
}





class fileHandle
{
private:
    string path;

public:
    vector<string> files;

    friend class CV;
    fileHandle(string path) : path(path) {};

    void listFiles()
    {
        string searchPath = path + "//*.*";

        WIN32_FIND_DATAA findFileData;
        HANDLE hFind = FindFirstFileA(searchPath.c_str(), &findFileData);

        if (hFind != INVALID_HANDLE_VALUE)
        {
            do
            {
                if (!(findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
                {
                    string fileName = findFileData.cFileName;
                    string extension = fileName.substr(fileName.find_last_of(".") + 1);

                    if (extension == "jpg" || extension == "png")
                    {
                        string filePath = path + "//" + fileName; // Construct complete file path
                        files.push_back(filePath); // Add complete file path to vector
                    }
                }
                else
                {
                    if (strcmp(findFileData.cFileName, ".") != 0 && strcmp(findFileData.cFileName, "..") != 0)
                    {
                        string subDirPath = path + "//" + findFileData.cFileName;
                        fileHandle subDir(subDirPath);
                        subDir.listFiles(); // Recursive call on subdirectory
                        files.insert(files.end(), subDir.files.begin(), subDir.files.end()); // Merge subdirectory files with current files
                    }
                }
            } while (FindNextFileA(hFind, &findFileData) != 0);
            FindClose(hFind);
        }
        else
        {
            cerr << "Could not open directory." << endl;
        }
    }



    void displayFiles()
    {
        for (const string& file : files)
        {
            cout << file << endl;
        }
    }
    vector<string> getFiles() const
    {
        return files;
    }

};



class Rectangle {
public:

    static int randrange(int min, int max) {
        range = max - min;
        return rand() % range + min;
    }

    static void zoom(int x, int y, int& u1, int& u2, int& v1, int& v2, float crop = 0.75) {
        u1 = randrange(0, (1 - crop) * x);
        u2 = u1 + crop * x;
        v1 = randrange(0, (1 - crop) * y);
        v2 = v1 + crop * y;

    }

};

class BasicCV
{
public:


    static void addGaussianNoise(const string& imagePath, double mean, double stddev)
    {
        Mat image = imread(imagePath);
        if (image.empty())
        {
            cout << "Error loading the image." << endl;
            return;
        }

        Mat noise(image.size(), CV_8UC3);
        randn(noise, Scalar(mean), Scalar(stddev));

        Mat noisyImage;
        add(image, noise, noisyImage);

        // Clip values to fit within the 0-255 range
        noisyImage.setTo(Scalar(0), noisyImage < 0);
        noisyImage.setTo(Scalar(255), noisyImage > 255);

        string newFilename = imagePath.substr(0, imagePath.find_last_of('.')) + "_Noisy.jpg";
        imwrite(newFilename, noisyImage);
    }


    static void equalizeHistogram(const string& imagePath)
    {
        Mat image = imread(imagePath);
        if (image.empty())
        {
            cout << "Error loading the image." << endl;
            return;
        }

        Mat equalizedImage;
        cvtColor(image, equalizedImage, COLOR_BGR2YCrCb);

        vector<Mat> channels;
        split(equalizedImage, channels);

        equalizeHist(channels[0], channels[0]);

        merge(channels, equalizedImage);
        cvtColor(equalizedImage, equalizedImage, COLOR_YCrCb2BGR);

        string newFilename = imagePath.substr(0, imagePath.find_last_of('.')) + "_Equalized.jpg";
        imwrite(newFilename, equalizedImage);
    }

    static void invertAndIncreaseContrast(const string& imagePath, float contrast_factor)
    {
        Mat image = imread(imagePath);
        if (image.empty())
        {
            cout << "Error loading the image." << endl;
            return;
        }

        Mat invertedImage = 255 - image;

        Mat contrastAdjusted;
        invertedImage.convertTo(contrastAdjusted, -1, contrast_factor, 0);

        string newFilename = imagePath.substr(0, imagePath.find_last_of('.')) + "_Inverted_Contrast.jpg";
        imwrite(newFilename, contrastAdjusted);
    }


    static void shiftColors(const string& imagePath, int hueShift, int saturationShift, int valueShift)
    {
        Mat image = imread(imagePath);
        if (image.empty())
        {
            cout << "Error loading the image." << endl;
            return;
        }

        Mat hsvImage;
        cvtColor(image, hsvImage, COLOR_BGR2HSV);

        for (int i = 0; i < hsvImage.rows; ++i)
        {
            for (int j = 0; j < hsvImage.cols; ++j)
            {
                hsvImage.at<Vec3b>(i, j)[0] = (hsvImage.at<Vec3b>(i, j)[0] + hueShift) % 180; // Hue
                hsvImage.at<Vec3b>(i, j)[1] = saturate_cast<uchar>(hsvImage.at<Vec3b>(i, j)[1] + saturationShift); // Saturation
                hsvImage.at<Vec3b>(i, j)[2] = saturate_cast<uchar>(hsvImage.at<Vec3b>(i, j)[2] + valueShift); // Value
            }
        }

        cvtColor(hsvImage, image, COLOR_HSV2BGR);

        string newFilename = imagePath.substr(0, imagePath.find_last_of('.')) + "_ShiftedColors.jpg";
        imwrite(newFilename, image);
    }

    static void rotateAndSaveImage(const string& imagePath, double angleDegrees = 45) {
        Mat image = imread(imagePath);
        if (image.empty()) {
            cout << "Error loading the image." << endl;
            return;
        }

        // Convert the angle to radians (required by OpenCV)
        double angleRadians = angleDegrees * CV_PI / 180.0;

        // Get the rotation matrix for the specified angle
        Point2f center(static_cast<float>(image.cols / 2), static_cast<float>(image.rows / 2));
        Mat rotationMatrix = getRotationMatrix2D(center, angleDegrees, 1);

        // Apply the rotation
        warpAffine(image, image, rotationMatrix, image.size());

        // Create a new filename with "_Rotated" appended before the file extension
        string newFilename = imagePath.substr(0, imagePath.find_last_of('.')) + "_Rotated.jpg";

        // Save the rotated image
        imwrite(newFilename, image);
    }




    static void flipImage(const string& imagePath)
    {
        Mat image = imread(imagePath);
        if (image.empty())
        {
            cout << "Error loading the image." << endl;
            return;
        }

        flip(image, image, 1); // 1 for horiz

        string newFilename = imagePath.substr(0, imagePath.find_last_of('.')) + "_Flipped.jpg";
        imwrite(newFilename, image);
    }



    static void crop(const string& imagePath, int x1, int x2, int y1, int y2) {
        Mat image = imread(imagePath);

        if (!image.data) {
            cout << "Error: Could not open or find the image." << endl;
            return;
        }

        Rect region(x1, y1, x2 - x1, y2 - y1);
        Mat croppedImage = image(region);

        string outputImagePath = imagePath.substr(0, imagePath.find_last_of(".")) + "_crop" + imagePath.substr(imagePath.find_last_of("."));
        imwrite(outputImagePath, croppedImage);
    }
    static void rotateAndSave(const string& imagePath)
    {
        Mat image = imread(imagePath);
        if (image.empty())
        {
            cout << "Error loading the image." << endl;
            return;
        }

        for (int i = 1; i <= 3; i++)
        {
            Mat rotatedImage;
            rotate(image, rotatedImage, ROTATE_90_CLOCKWISE); // Rotate 90 degrees clockwise

            string newFilename = imagePath.substr(0, imagePath.find_last_of('.')) + "_rotated_" + to_string(i * 90) + ".jpg";
            imwrite(newFilename, rotatedImage);

            image = rotatedImage; // Set the rotated image as the new base for the next rotation
        }
    }
};



static void getImageDimensions(const string& imagePath, int& width, int& height) {
    Mat image = imread(imagePath);

    if (!image.data) {
        cout << "Error: Could not open or find the image." << endl;
        width = -1;
        height = -1;
        return;
    }

    width = image.cols;
    height = image.rows;
}

void printProgressBar(int progress, int total) {
    float percentage = (float)progress / total * 100.0;
    int barWidth = 70;

    std::cout << "[";
    int pos = barWidth * percentage / 100.0;
    for (int i = 0; i < barWidth; ++i) {
        if (i < pos) std::cout << "=";
        else if (i == pos) std::cout << ">";
        else std::cout << " ";
    }
    std::cout << "] " << std::fixed << std::setprecision(2) << percentage << " %\r";
    std::cout.flush();
}

void processImage(const string& imagePath) {
    int h, w;
    int u1, u2, v1, v2;
    getImageDimensions(imagePath, w, h);

    Rectangle::zoom(w, h, u1, u2, v1, v2);
    printProgressBar(progress, total_progress);
    progress++;
    BasicCV::crop(imagePath, u1, u2, v1, v2);
    printProgressBar(progress, total_progress);
    progress++;
    BasicCV::equalizeHistogram(imagePath);
    printProgressBar(progress, total_progress);
    progress++;
    BasicCV::rotateAndSaveImage(imagePath);
    printProgressBar(progress, total_progress);
    progress++;
    BasicCV::flipImage(imagePath);
    printProgressBar(progress, total_progress);
    progress++;
    BasicCV::addGaussianNoise(imagePath, 100, 1000);
    printProgressBar(progress, total_progress);
    progress++;
}



int main() {
    srand(time(NULL));
    int choice = getUserSelectedOpt();
    cout << "Choice: " << choice << endl;
    string xpath = SelectFolder();
    fileHandle fh(xpath);
    fh.listFiles();
    total_progress = (fh.getFiles().size()) * 6;
    for (const string& file : fh.getFiles()) {
        processImage(file);
    }
    return 0;
}
