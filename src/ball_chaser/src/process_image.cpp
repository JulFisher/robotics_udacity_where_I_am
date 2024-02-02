#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include <math.h>
#include <limits>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/highgui.hpp>


// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    if (!client.call(srv)) ROS_ERROR("Failed to call service ball_chaser");
}

float check_direction(uint32_t width, uint32_t position_in_line)
{
    if (0 <= position_in_line < static_cast<uint32_t>(width*0.45)) {return 1.f;}
    else if (static_cast<uint32_t>(width*0.45) <= position_in_line <= static_cast<uint32_t>(width*0.55))
    {
        return 0.f;
    }
    else {return -1.f;}
}


// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{

    int white_pixel = 255;
    float linear_x{0.3};
    float angular_z{0.2};
    float direction_multiplicator = std::numeric_limits<float>::infinity();

    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera

    //camera properties: height = 800; width = 800; encoding = R8G8B8; step = 2400
    
    cv_bridge::CvImagePtr cv_ptr;
    cv_ptr = cv_bridge::toCvCopy(img, img.encoding);
    cv_bridge::CvImagePtr one_channel_img = cv_bridge::cvtColor(cv_ptr, "mono8");
    for (int row = 0;  row < one_channel_img->image.rows ; ++row) {
        //uchar* pixel = one_channel_img->image.ptr(row);
        for (int col = 0; col < one_channel_img->image.cols; ++col) {
            if (one_channel_img->image[row][col] == white_pixel) {
                direction_multiplicator = check_direction(img.width, col);
                break;
                }
            }
        }
    if (std::isinf(direction_multiplicator)){
        drive_robot(0.0,0.0);
    }
    else
        {drive_robot(linear_x, angular_z*direction_multiplicator);}

}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}