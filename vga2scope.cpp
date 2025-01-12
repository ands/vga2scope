#include <opencv2/opencv.hpp> // OpenCV library for image processing
#include <iostream>          // For console input and output
#include <cmath>             // For mathematical computations (e.g., square root)

// Main function
int main(int argc, char **argv)
{
    // Check for command-line arguments
    if (argc < 6) // At least 5 parameters are required
    {
        std::cout << "Usage: " << argv[0] << " <video> <vga window x> <vga window y> <vga width> <vga height>\n";
        std::cout << "Usage: " << argv[0] << " <camera id> <vga window x> <vga window y> <vga width> <vga height>\n";
        return 0;
    }

    // Read input parameters from the command line
    const std::string filename = argv[1];   // Filename or camera ID
    int vgaX = std::stoi(argv[2]);          // X-coordinate of VGA window
    int vgaY = std::stoi(argv[3]);          // Y-coordinate of VGA window
    int vgaWidth = std::stoi(argv[4]);      // Width of VGA window
    int vgaHeight = std::stoi(argv[5]);     // Height of VGA window
    int outputPixelCount = vgaWidth * vgaHeight; // Total number of pixels in the VGA output window

    // Initialize the VideoCapture object (to load video or camera)
    cv::VideoCapture cap;

    // Check if the input is a numeric value (for camera ID)
    if (isdigit(filename[0]))
        cap.open(std::stoi(filename)); // Open camera by ID
    else
        cap.open(filename); // Open video file

    // Verify if the camera/file was successfully opened
    if (!cap.isOpened())
    {
        std::cerr << "Could not open file/camera!\n";
        return 1;
    }

    // Initialize OpenCV matrices and windows
    cv::Mat frame, edges, lines, out(cv::Size(vgaWidth, vgaHeight), CV_8UC3, cv::Scalar(0, 0, 0)); // Initialize the output matrix

    // Position output windows
    cv::namedWindow("frame", cv::WINDOW_AUTOSIZE);  // Original frame
    cv::moveWindow("frame", 0, 32);
    cv::namedWindow("edges", cv::WINDOW_AUTOSIZE);  // Edge-detected image
    cv::moveWindow("edges", 640, 32);
    cv::namedWindow("lines", cv::WINDOW_AUTOSIZE);  // Contour-drawn image
    cv::moveWindow("lines", 1280, 32);
    cv::namedWindow("out", cv::WINDOW_AUTOSIZE);    // Output image
    cv::moveWindow("out", vgaX, vgaY);

    // Main processing loop
    while (true)
    {
        cap >> frame; // Read the next frame from video/camera
        if (frame.empty() || (cv::waitKey(1) & 0xFF) == 'q') // Exit if no frame or 'q' is pressed
            break;

        cv::imshow("frame", frame); // Display the original frame

        // Perform edge detection
        cv::cvtColor(frame, edges, cv::COLOR_BGR2GRAY); // Convert to grayscale
        cv::Canny(edges, edges, 128.0, 130.0, 3); // Apply Canny edge detection
        cv::imshow("edges", edges); // Display the edge-detected image

        // Find contours
        std::vector<std::vector<cv::Point>> contours; // Store the detected contours
        cv::findContours(edges, contours, cv::RETR_LIST, cv::CHAIN_APPROX_SIMPLE); // Extract contours from edges
        lines = cv::Mat::zeros(frame.size(), CV_8UC3); // Empty image for contour visualization
        cv::drawContours(lines, contours, -1, cv::Scalar(32, 255, 32), 1); // Draw all contours onto the image
        cv::imshow("lines", lines); // Display the contour-drawn image

        // Calculate the total length of all contours
        float contourLengthSum = 0.0f;
        for (const auto& contour : contours) // Iterate through each contour
        {
            for (size_t i = 0; i < contour.size() - 1; ++i) // Traverse points in the contour
            {
                float dx = contour[i + 1].x - contour[i].x; // Difference in x-coordinate
                float dy = contour[i + 1].y - contour[i].y; // Difference in y-coordinate
                contourLengthSum += std::sqrt(dx * dx + dy * dy); // Add Euclidean distance between points
            }
        }

        // Compute the scaling factor
        float factor = static_cast<float>(outputPixelCount) / contourLengthSum;

        // Generate the output image
        int cx = 0, cy = 0; // Starting position in the output matrix
        float xScale = 255.0f / frame.cols; // Scaling factor for x
        float yScale = 255.0f / frame.rows; // Scaling factor for y
        for (const auto& contour : contours)
        {
            for (size_t i = 1; i < contour.size(); ++i)
            {
                float x0x1 = contour[i].x - contour[i - 1].x; // Difference in x
                float y0y1 = contour[i].y - contour[i - 1].y; // Difference in y
                int n = static_cast<int>(std::sqrt(x0x1 * x0x1 + y0y1 * y0y1) * factor); // Number of points based on segment length
                float x = static_cast<float>(contour[i - 1].x) * xScale; // Scaled start x
                float y = 255.0f - static_cast<float>(contour[i - 1].y) * yScale; // Scaled start y
                float dx = x0x1 * xScale / (n - 1); // Step size in x
                float dy = -y0y1 * yScale / (n - 1); // Step size in y

                for (int j = 0; j < n; ++j)
                {
                    if (cy >= vgaHeight) // If height is exceeded, exit
                        break;
                    out.at<cv::Vec3b>(cy, cx) = cv::Vec3b(0, static_cast<unsigned char>(y), static_cast<unsigned char>(x)); // Set pixel value
                    x += dx; // Update x-coordinate
                    y += dy; // Update y-coordinate
                    if (++cx == vgaWidth) // Move to the next row
                    {
                        cx = 0;
                        if (++cy == vgaHeight) // If height is reached, exit
                            break;
                    }
                }
            }
        }

        cv::imshow("out", out); // Display the output image

        // Optional synchronization with video playback
        // if (!cap.get(cv::CAP_PROP_FRAME_COUNT)) cv::waitKey(30);
    }

    return 0; // End of program
}
