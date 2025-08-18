#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>

class Course {
public:
    std::string courseNumber;
    std::string name;
    std::vector<std::string> prerequisites;
};

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
        std::getline(ss, course.courseNumber, ',');
        std::getline(ss, course.name, ',');

        // Read prerequisites and split them by commas
        std::string prereq;
        while (std::getline(ss, prereq, ',')) {
            course.prerequisites.push_back(prereq);
        }

        courses.push_back(course);
    }

    file.close();
}

void printCourseList(const std::vector<Course>& courses) {
    std::cout << "Course List:" << std::endl;
    for (const Course& course : courses) {
        std::cout << course.courseNumber << " - " << course.name << std::endl;
    }
}

void printCourseInfo(const std::vector<Course>& courses, const std::string& courseNumber) {
    for (const Course& course : courses) {
        if (course.courseNumber == courseNumber) {
            std::cout << "Course Number: " << course.courseNumber << std::endl;
            std::cout << "Course Name: " << course.name << std::endl;

            if (!course.prerequisites.empty()) {
                std::cout << "Prerequisites: ";
                for (const std::string& prereq : course.prerequisites) {
                    std::cout << prereq << ", ";
                }
                std::cout << std::endl;
            }
            else {
                std::cout << "No prerequisites for this course." << std::endl;
            }
            return; // Found and printed, no need to continue searching.
        }
    }

    std::cout << "Course not found: " << courseNumber << std::endl;
}

int main() {
    std::vector<Course> courses;

    int choice = 0;
    while (choice != 9) {
        std::cout << "Welcome to the course planner." << std::endl;
        std::cout << "1. Load Data Structure." << std::endl;
        std::cout << "2. Print Course List." << std::endl;
        std::cout << "3. Print Course." << std::endl;
        std::cout << "9. Exit" << std::endl;
        std::cout << "What would you like to do? ";
        std::cin >> choice;

        switch (choice) {
        case 1:
            loadDataStructure(courses);
            break;
        case 2:
            if (courses.empty()) {
                std::cout << "Please load data first." << std::endl;
            }
            else {
                printCourseList(courses);
            }
            break;
        case 3:
            if (courses.empty()) {
                std::cout << "Please load data first." << std::endl;
            }
            else {
                std::string courseNumber;
                std::cout << "What course do you want to know about? ";
                std::cin >> courseNumber;
                printCourseInfo(courses, courseNumber);
            }
            break;
        case 9:
            std::cout << "Thank you for using the course planner!" << std::endl;
            break;
        default:
            std::cout << choice << " is not a valid option." << std::endl;
        }
    }

    return 0;
}
