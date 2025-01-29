import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import GroupAction
from launch_ros.actions import Node, PushRosNamespace
import xacro


def generate_launch_description():

    urdf = os.path.join(
        get_package_share_directory('baxter_description'),
        # get_package_share_directory('baxter_description_legacy'),
        'urdf',
        # 'baxter.urdf.xacro')
        # 'baxter2.urdf.xacro')
        'baxter2_include.urdf.xacro')
    print("urdf path:", urdf)
    rsp = Node(package='robot_state_publisher',
            executable='robot_state_publisher',
            name='robot_state_publisher',
            output='screen',
            parameters=[{'robot_description': xacro.process(urdf)}])
    
    jsp = Node( #ja dodao, bez njega ti nece bit dobro vizualizirano (tj falit ce povezanost u tf treeju jer ne publishas info o zglobovima na /joint_states pa rsp ne moze publishat transformaciju)
            package='joint_state_publisher_gui',
            executable='joint_state_publisher_gui',
            name='joint_state_publisher_gui',
            output='screen',
            parameters=[{
                'robot_description': xacro.process(urdf)
            }]
        )

    # return LaunchDescription([GroupAction([PushRosNamespace('robot'), rsp])])
    return LaunchDescription([GroupAction([PushRosNamespace('robot'), rsp, jsp])])
