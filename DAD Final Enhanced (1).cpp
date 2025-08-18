#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

// =========================
// Class Definition
// =========================
class Course {
public:
    std::string courseNumber;              // Unique identifier for the course (e.g., CS101)
    std::string name;                      // Full course name (e.g., Introduction to CS)
    std::vector<std::string> prerequisites; // List of course numbers that are prerequisites
};

// =========================
// Function to Load Courses
// =========================
void loadDataStructure(std::vector<Course>& courses) {
    std::string fileName = "courses.txt";
    std::ifstream file(fileName);

    if (!file.is_open()) {
        std::cerr << "Failed to open the file: " << fileName << std::endl;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        Course course;
        std::istringstream ss(line);
        std::getline(ss, course.courseNumber, ','); // Read course number
        std::getline(ss, course.name, ',');         // Read course name

        // Read and store all prerequisites (if any)
        std::string prereq;
        while (std::getline(ss, prereq, ',')) {
            course.prerequisites.push_back(prereq);
        }

        courses.push_back(course); // Add course to the vector
    }

    file.close(); // Always close file after reading
}

// =========================
// Function to Print All Courses
// =========================
void printCourseList(const std::vector<Course>& courses) {
    std::cout << "\nCourse List:\n";
    for (const Course& course : courses) {
        std::cout << course.courseNumber << " - " << course.name << std::endl;
    }
}

// =========================
// Function to Print Info for One Course
// =========================
void printCourseInfo(const std::vector<Course>& courses, const std::string& courseNumber) {
    for (const Course& course : courses) {
        if (course.courseNumber == courseNumber) {
            std::cout << "\nCourse Number: " << course.courseNumber << std::endl;
            std::cout << "Course Name: " << course.name << std::endl;

            // Print prerequisites if they exist
            if (!course.prerequisites.empty()) {
                std::cout << "Prerequisites: ";
                for (const std::string& prereq : course.prerequisites) {
                    std::cout << prereq << " ";
                }
                std::cout << std::endl;
            }
            else {
                std::cout << "No prerequisites for this course." << std::endl;
            }
            return;
        }
    }

    // If course not found
    std::cout << "Course not found: " << courseNumber << std::endl;
}

// =========================
// NEW: Sorting Function - By Course Number
// =========================
void sortCoursesByNumber(std::vector<Course>& courses) {
    std::sort(courses.begin(), courses.end(), [](const Course& a, const Course& b) {
        return a.courseNumber < b.courseNumber; // Alphabetical comparison
        });
}

// =========================
// NEW: Sorting Function - By Course Name
// =========================
void sortCoursesByName(std::vector<Course>& courses) {
    std::sort(courses.begin(), courses.end(), [](const Course& a, const Course& b) {
        return a.name < b.name; // Alphabetical comparison
        });
}

// =========================
// NEW: User Interface to Choose Sort Type
// =========================
void sortMenu(std::vector<Course>& courses) {
    int sortChoice = 0;

    std::cout << "\nSort Options:\n";
    std::cout << "1. Sort by Course Number (e.g., CS101 < CS201)\n";
    std::cout << "2. Sort by Course Name (e.g., Algorithms < Programming)\n";
    std::cout << "Choose sorting option: ";
    std::cin >> sortChoice;

    // Apply user's choice
    if (sortChoice == 1) {
        sortCoursesByNumber(courses);
        std::cout << "Courses sorted by course number.\n";
    }
    else if (sortChoice == 2) {
        sortCoursesByName(courses);
        std::cout << "Courses sorted by course name.\n";
    }
    else {
        std::cout << "Invalid choice. No sorting applied.\n";
    }
}

// =========================
// Main Program Loop
// =========================
int main() {
    std::vector<Course> courses; // Main data structure to store courses

    int choice = 0;
    while (choice != 9) {
        std::cout << "\n=== Course Planner Menu ===\n";
        std::cout << "1. Load Data Structure\n";
        std::cout << "2. Print Course List\n";
        std::cout << "3. Print Course\n";
        std::cout << "4. Sort Courses (NEW)\n"; // NEW menu option
        std::cout << "9. Exit\n";
        std::cout << "What would you like to do? ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            loadDataStructure(courses); // Load from file
            std::cout << "Data loaded.\n";
            break;
        case 2:
            if (courses.empty()) {
                std::cout << "Please load data first.\n";
            }
            else {
                printCourseList(courses); // Print unsorted/sorted list
            }
            break;
        case 3:
            if (courses.empty()) {
                std::cout << "Please load data first.\n";
            }
            else {
                std::string courseNumber;
                std::cout << "Enter course number: ";
                std::cin >> courseNumber;
                printCourseInfo(courses, courseNumber); // Search and display course
            }
            break;
        case 4:
            if (courses.empty()) {
                std::cout << "Please load data first.\n";
            }
            else {
                sortMenu(courses); // Call new sort UI
            }
            break;
        case 9:
            std::cout << "Exiting. Goodbye!\n";
            break;
        default:
            std::cout << choice << " is not a valid option.\n";
        }
    }

    return 0;
}
