#include <baxter_bridge/solve_ik.h>
#include <baxter_bridge/bridge_1to2.h>
#include <baxter_bridge/topic_poller.h>
#include <rclcpp/logging.hpp>
#include <rclcpp/rclcpp.hpp>
#include <baxter_bridge/srv/exists.hpp>
#include <baxter_bridge/srv/open.hpp>

using namespace baxter_bridge;
using namespace baxter_bridge::srv;

int main(int argc, char** argv)
{
  const auto [ok,on_baxter,is_static] = Bridge::init(argc, argv); {}
  if(!ok)
    return 0;

  std::unique_ptr<TopicPoller> poller;
  if(on_baxter)
  {
    RCLCPP_INFO(Bridge::ros2()->get_logger(), "Connected to real Baxter");
    // other cheap bridges we are usually interested in
    // makes it easier to spawn in RViz2 if they are advertized in ROS 2
    Factory::createBridge("/robot/range/left_hand_range/state");
    Factory::createBridge("/robot/range/right_hand_range/state");
    Factory::createBridge("/robot/sonar/head_sonar/state");
    Factory::createBridge("/robot/joint_states");

    //moje
    Factory::createBridge("/robot/limb/left/follow_joint_trajectory_bridge"); //moje za slanje action goal poruke sa ros2 na ros1
    Factory::createBridge("/robot/limb/right/follow_joint_trajectory_bridge"); //moje za slanje action goal poruke sa ros2 na ros1
    Factory::createBridge("/moj_2to1_debug_topic"); //moje za debug
    Factory::createBridge("/moj_1to2_debug_topic"); //moje za debug
    Factory::createBridge("/joint_states"); //moje da bi mi mogli rviz, moveit itd koristit joint_states od pravog baxtera


    if(is_static)
    {
      RCLCPP_INFO(Bridge::ros2()->get_logger(), "[MOJE] main(), bridge is static, creating remaining bridges");
      Factory::createRemainingBridges();
    }
    else
    {
      poller = std::make_unique<TopicPoller>(Bridge::ros2());
      RCLCPP_INFO(Bridge::ros2()->get_logger(), "[MOJE] main(), poller created");
    }
  }
  else
  {
    RCLCPP_WARN(Bridge::ros2()->get_logger(), "Not connected to real Baxter");
  }

  // basic command and inverse kinematics for both arms
  SolveIK left("left"), right("right"); //kreiraju /robot/limb/left/joint_command i /robot/limb/right/joint_command bridge-eve

  // service to spawn a topic on demand
  auto open_srv = Bridge::ros2()->create_service<Open>("bridge_open",
                                                       [&](const Open::Request::SharedPtr req, Open::Response::SharedPtr res)
  {res->open = Factory::createBridge(req->topic);});

  auto exist_srv = Bridge::ros2()->create_service<Exists>("bridge_exists",
                                                          [&](const Exists::Request::SharedPtr req, Exists::Response::SharedPtr res)
  {
    const auto direction{Factory::exists(req->topic)};
    if(!direction.has_value())
      res->direction = res->NONE;
    else if(direction.value() == Bridge::Direction::ROS_1_TO_2)
      res->direction = res->ROS_1_TO_2;
    else
      res->direction = res->ROS_2_TO_1;
    return;
  });


  ros::AsyncSpinner async(1);
  async.start();

  while(ros::ok() && rclcpp::ok())
  {
    Bridge::spinOnce();

    // create new bridges if needed
    if(poller)
    {
      for(const auto &topic: poller->pendingBridges())
      {
        RCLCPP_INFO(Bridge::ros2()->get_logger(), "  [MOJE] main(), creating bridge for topic %s", topic.c_str());
        bool result = Factory::createBridge(topic);
        if(result)
        {
         RCLCPP_INFO(Bridge::ros2()->get_logger(), "  [MOJE] main(), bridge for topic %s successfully created", topic.c_str()); 
        }
        else
        {
          RCLCPP_ERROR(Bridge::ros2()->get_logger(), "  [MOJE] main(), bridge for topic %s wasn't created successfully", topic.c_str());
        }
      }
    }
  }

  rclcpp::shutdown();
  ros::shutdown();
}
