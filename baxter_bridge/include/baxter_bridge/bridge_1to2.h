#ifndef BAXTER_BRIDGE_1TO2_H
#define BAXTER_BRIDGE_1TO2_H

#include <baxter_bridge/bridge.h>
#include <baxter_bridge/conversions.h>

namespace baxter_bridge
{

template<class Msg1, class Msg2>
struct Bridge_1to2 : public Bridge
{
  Bridge_1to2(const std::string &topic)
  {
    pub = ros2()->create_publisher<Msg2>(topic, 10);
    // pub = ros2()->create_publisher<Msg2>(topic != "/robot/joint_states" ? topic : "/joint_states", 10); //zelim da mi se /robot/joint_states sa ros1 publisha na /joint_states na ros2 kako bi mi bilo kompatibilno sa ostatkom moveit koda
    sub = ros1()->subscribe<Msg1>(topic, 10, [&](const typename Msg1::ConstPtr &msg)
    {
      // ////moj debug
      // if (topic == "/robot/joint_states")
      // {
      //   std::cout << "[MOJE] ros1 got message on topic: " << topic << std::endl;
      // }
      // ////

      static Msg2 msg2;
      convert(*msg, msg2);
      pub->publish(msg2);
    });
  }

  inline std::string topic() const override
  {
    return sub.getTopic();
  }

  inline Direction direction() const override
  {
    return Direction::ROS_1_TO_2;
  }

private:
  // ros 1
  ros::Subscriber sub;

  // ros 2
  typename rclcpp::Publisher<Msg2>::SharedPtr pub;
};

}

#endif
