#include <baxter_bridge/topic_poller.h>
#include <baxter_bridge/factory.h>

namespace baxter_bridge
{

TopicPoller::TopicPoller(rclcpp::Node* node2) : node2{node2}
{
  poller = node2->create_wall_timer(std::chrono::seconds(1), [&](){poll();});
}

void TopicPoller::poll()
{
  const auto info1 = []()
                    {
                      ros::master::V_TopicInfo info;
                      ros::master::getTopics(info); //dohvaca sve topice koji se poublishaju (ros1)
                      return info; //vector parova topic_name, topic_type
                    }();

  const auto info2{node2->get_topic_names_and_types()};

  ros1_published.clear();
  ros1_subscribed.clear();
  ros2_published.clear();
  ros2_subscribed.clear();

  for(const auto &info: info1) //gleda ako na taj topic koji postoji (koji je trenutno available) u ros1 sustavu baxter publisha ili subscribea, ako ni jedno ni drugo onda nista jer nije potreban bridge
  {
    const auto &topic{info.name};
    if(Factory::isPublishedByBaxter(topic))
      ros1_published.push_back(topic); //trenutno aktivni topici iz ros1 koje baxter publisha,, baxter je u ros1 sustavu pa ih ros1 publisha
    else if(Factory::isSubscribedByBaxter(topic))
      ros1_subscribed.push_back(topic); //trenutno aktivni topici iz ros1 na koje se baxter subscribea,, baxter je u ros1 sustavu pa se ros1 subscribea na njih
  }

  for(const auto &[topic, msg]: info2) //gleda ako na taj topic koji postoji (koji je trenutno available) u ros2 sustavu baxter publisha ili subscribea, ako ni jedno ni drugo onda nista jer nije potreban bridge
  {
    if(Factory::isPublishedByBaxter(topic))
      ros2_subscribed.push_back(topic); //ako se taj topic publisha, a publisha ga ros1 (jer je baxter u ros1) onda se ros2 mora subscribeat na njega
    else if(Factory::isSubscribedByBaxter(topic))
      ros2_published.push_back(topic); //ako se na taj topic subrscribea, a subscribea se ros1 (jer je baxter u ros1) onda ga ros2 mora publishat
  }

  std::sort(ros1_published.begin(), ros1_published.end());
  std::sort(ros1_subscribed.begin(), ros1_subscribed.end());
  std::sort(ros2_published.begin(), ros2_published.end());
  std::sort(ros2_subscribed.begin(), ros2_subscribed.end());

}

std::vector<std::string> TopicPoller::pendingBridges() const
{
  std::vector<std::string> pending;

  std::set_intersection(ros1_published.begin(),ros1_published.end(),
                            ros2_subscribed.begin(),ros2_subscribed.end(),
                            back_inserter(pending));
  std::set_intersection(ros2_published.begin(),ros2_published.end(),
                            ros1_subscribed.begin(),ros1_subscribed.end(),
                            back_inserter(pending));
  return pending;
}
}
