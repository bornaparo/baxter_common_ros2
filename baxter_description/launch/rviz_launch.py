import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node
from launch.conditions import IfCondition


def generate_launch_description():

    config = os.path.join(
        get_package_share_directory('baxter_description'),
        'launch',
        'config.rviz')

    if os.environ['ROS_DISTRO'] in ('foxy', 'galactic'):
        args = "0 0 .9 0 0 0 ground base"
    else:
        args = "--z .9 --frame-id ground --child-frame-id base" #base je u odnosu na ground pomaknut za 0.9 po ground z osi,, da bi dosao od grounda do base-a moras ici za 0.9m po ground z osi
    # args = "0 0 .9 0 0 0 ground base"

    return LaunchDescription([
        Node(
            package='rviz2',
            executable='rviz2',
            arguments=['-d',config]
            ),
        # Node( #nije potrebno jer sam dodao link (i joint) u urdf
        #     package='tf2_ros',
        #     executable='static_transform_publisher',
        #     arguments=args.split()
        #     ),
        ])
