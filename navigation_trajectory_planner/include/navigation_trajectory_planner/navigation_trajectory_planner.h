#ifndef NAVIGATION_TRAJECTORY_PLANNER_H
#define NAVIGATION_TRAJECTORY_PLANNER_H

#include "navigation_trajectory_msgs/SampleValidPoses.h"
#include "navigation_trajectory_planner/environment_navxythetalat_generic.h"

#include <costmap_2d/costmap_2d_ros.h>
#include <sbpl/headers.h>
#include <nav_core/base_global_planner.h>
#include "move_base_trajectory/base_global_planner.h"
#include <ros/ros.h>
#include <nav_msgs/Path.h>

#include <iostream>
#include <vector>

namespace navigation_trajectory_planner
{

class XYThetaStateChangeQuery : public StateChangeQuery
{
public:
    XYThetaStateChangeQuery(EnvironmentNavXYThetaLatGeneric* env, const std::vector<nav2dcell_t> & changedcells);

    virtual const std::vector<int> * getPredecessors() const;
    virtual const std::vector<int> * getSuccessors() const;

public:
    EnvironmentNavXYThetaLatGeneric* env_;
    std::vector<nav2dcell_t> changedcells_;
    mutable std::vector<int> predsOfChangedCells_;
    mutable std::vector<int> succsOfChangedCells_;
};

class NavigationTrajectoryPlanner : public move_base_trajectory::BaseGlobalPlannerTrajectory
{
public:
    NavigationTrajectoryPlanner();
    virtual ~NavigationTrajectoryPlanner() {
        delete private_nh_;
    }

    virtual void initialize(std::string name, costmap_2d::Costmap2DROS* costmap_ros);

    /// Main query from move_base
    virtual bool makeTrajectory(const geometry_msgs::PoseStamped& start, const geometry_msgs::PoseStamped& goal, moveit_msgs::RobotTrajectory & traj);

    /// Returns the frame that all planning is happening in.
    /**
     * This is usually the planning frame of the environment. All data has to be either in this
     * frame or has to be transformed to this.
     */
    virtual std::string getPlanningFrame() const;

    /// Read in parameters that can be re-set for each makePlan call.
    virtual void readDynamicParameters();

protected:
    virtual bool sampleValidPoses(navigation_trajectory_msgs::SampleValidPoses::Request & req, navigation_trajectory_msgs::SampleValidPoses::Response & resp);

    /// Make sure that a ready to use environment exists.
    virtual bool createAndInitializeEnvironment();

    /// Make sure there is a planner instance.
    virtual bool createPlanner();

    /// Create a custom environment for this planner.
    virtual EnvironmentNavXYThetaLatGeneric* createEnvironment(ros::NodeHandle & nhPriv) = 0;
    virtual bool initializeEnvironment(
            double trans_vel, double timeToTurn45Degs, const std::string & motion_primitive_filename) = 0;

    /// Update internal representation of the planner for a plan request.
    /**
     * Called, whenever makePlan is called. Start and Goal state have already been set and
     * env_->updateForPlanRequest() has also been called.
     *
     * This object will be deleted after the query.
     *
     * If possible should return a XYThetaStateChangeQuery that can be passed to the search algorithm.
     * If NULL is returned, the planner will plan from scratch.
     */
    virtual XYThetaStateChangeQuery* updateForPlanRequest() { return NULL; }

    virtual void publishStats(int solution_cost, int solution_size, const geometry_msgs::PoseStamped& start, const geometry_msgs::PoseStamped& goal);
    virtual void publish_expansions();
    virtual void publish_expansion_map();

    nav_msgs::Path trajectoryToGuiPath(const moveit_msgs::RobotTrajectory & traj) const;

    virtual void fillGrid(nav_msgs::OccupancyGrid & grid, const std::vector< std::set<int> > & gridDirections, int maxDirections);

protected:
    bool initialized_;
    ros::NodeHandle* private_nh_;

    SBPLPlanner* planner_;
    EnvironmentNavXYThetaLatGeneric* env_;

    double allocated_time_;
    double initial_epsilon_;
    int force_scratch_limit_;   ///< if the number of changed cells is >= this, planning from scratch will happen

    costmap_2d::Costmap2DROS* costmap_ros_;

    ros::Publisher plan_pub_;
    ros::Publisher traj_pub_;
    ros::Publisher stats_publisher_;
    ros::Publisher expansions_publisher_;

    std::string expansion_color_scheme_;    ///< occupancy or costmap
    ros::Publisher pub_expansion_map_;
    ros::Publisher pub_generation_map_;
    ros::Publisher pub_expansion_first_map_;
    ros::Publisher pub_generation_first_map_;

    ros::ServiceServer srv_sample_poses_;
};

}

#endif

