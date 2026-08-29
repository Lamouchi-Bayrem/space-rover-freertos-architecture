#include <algorithm>
#include <geometry_msgs/msg/twist.hpp>
#include <rclcpp/rclcpp.hpp>
class RoverControl: public rclcpp::Node { public: RoverControl():Node("rover_control_node") {
 sub_=create_subscription<geometry_msgs::msg::Twist>("cmd_vel",10,[this](geometry_msgs::msg::Twist::SharedPtr m){
  const double linear=std::clamp(m->linear.x,-1.0,1.0); const double angular=std::clamp(m->angular.z,-2.0,2.0);
  RCLCPP_INFO(get_logger(),"bounded command linear=%.2f angular=%.2f",linear,angular); }); }
 private:rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr sub_; };
int main(int argc,char **argv){rclcpp::init(argc,argv);rclcpp::spin(std::make_shared<RoverControl>());rclcpp::shutdown();}
