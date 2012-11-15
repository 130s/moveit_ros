/*
 * Copyright (c) 2008, Willow Garage, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the Willow Garage, Inc. nor the names of its
 *       contributors may be used to endorse or promote products derived from
 *       this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* Author: Ioan Sucan */

#ifndef MOVEIT_ROBOT_INTERACTION_ROBOT_INTERACTION_
#define MOVEIT_ROBOT_INTERACTION_ROBOT_INTERACTION_

#include <visualization_msgs/InteractiveMarkerFeedback.h>
#include <planning_models/kinematic_state.h>
#include <boost/function.hpp>
#include <tf/tf.h>

namespace interactive_markers
{
class InteractiveMarkerServer;
}

namespace robot_interaction
{
  
class RobotInteraction
{
public:
  
  /// The topic name on which the internal Interactive Marker Server operates
  static const std::string INTERACTIVE_MARKER_TOPIC;
  
  struct EndEffector
  {
    /// The name of the group that sustains the end-effector (usually an arm)
    std::string parent_group;

    /// The name of the link in the parent group that connects to the end-effector
    std::string parent_link;

    /// The name of the group that defines the group joints
    std::string eef_group;
    
    /// The size of the end effector group (diameter of enclosing sphere)
    double size;
  };

  struct VirtualJoint
  { 
    /// The link in the robot model this joint connects to
    std::string connecting_link;
    
    /// The name of the virtual joint
    std::string joint_name;

    /// The number of DOF this virtual joint has
    unsigned int dof;

    /// The size of the connectig link  (diameter of enclosing sphere)
    double size;
  };
  
  class InteractionHandler
  {
  public:
    InteractionHandler(const std::string &name,
                       const planning_models::KinematicState &kstate,
                       const boost::shared_ptr<tf::Transformer> &tf = boost::shared_ptr<tf::Transformer>());
    InteractionHandler(const std::string &name,
                       const planning_models::KinematicModelConstPtr &kmodel,
                       const boost::shared_ptr<tf::Transformer> &tf = boost::shared_ptr<tf::Transformer>());
    
    virtual ~InteractionHandler(void)
    {
    }
    
    const std::string& getName(void) const
    {
      return name_;
    }
    
    const planning_models::KinematicStatePtr& getState(void) const
    {
      return kstate_;
    }
    
    void setState(const planning_models::KinematicState& kstate)
    {
      *kstate_ = kstate;
    }    
    
    void setUpdateCallback(const boost::function<void(InteractionHandler*)> &callback)
    {
      update_callback_ = callback;
    }
    
    virtual void handleEndEffector(const RobotInteraction::EndEffector& eef, const visualization_msgs::InteractiveMarkerFeedbackConstPtr &feedback);
    virtual void handleVirtualJoint(const RobotInteraction::VirtualJoint& vj, const visualization_msgs::InteractiveMarkerFeedbackConstPtr &feedback);
    virtual bool inError(const RobotInteraction::EndEffector& eef);
    virtual bool inError(const RobotInteraction::VirtualJoint& vj);
    
  protected:

    bool transformFeedbackPose(const visualization_msgs::InteractiveMarkerFeedbackConstPtr &feedback, geometry_msgs::PoseStamped &tpose);

    std::string name_;
    planning_models::KinematicStatePtr kstate_;
    boost::shared_ptr<tf::Transformer> tf_;
    std::set<std::string> error_state_;
    boost::function<void(InteractionHandler*)> update_callback_;
    
  private:
    
    void setup(void);
  };

  typedef boost::shared_ptr<InteractionHandler> InteractionHandlerPtr;
  typedef boost::shared_ptr<const InteractionHandler> InteractionHandlerConstPtr;
  
  RobotInteraction(const planning_models::KinematicModelConstPtr &kmodel);
  ~RobotInteraction(void);
  
  void decideActiveComponents(const std::string &group);
  void decideActiveEndEffectors(const std::string &group);
  void decideActiveVirtualJoints(const std::string &group);
  
  void clear(void);
  
  void addInteractiveMarkers(const InteractionHandlerPtr &handler);
  void publishInteractiveMarkers(void);
  void clearInteractiveMarkers(void);
  
  const std::vector<EndEffector>& getActiveEndEffectors(void) const
  {
    return active_eef_;
  }

  const std::vector<VirtualJoint>& getActiveVirtualJoints(void) const
  {
    return active_vj_;
  }
  
  static bool updateState(planning_models::KinematicState &state, const EndEffector &eef, const geometry_msgs::Pose &pose, double ik_timeout = 0.1);
  static bool updateState(planning_models::KinematicState &state, const VirtualJoint &vj, const geometry_msgs::Pose &pose);

private:
  
  // return the diameter of the sphere that certainly can enclose the AABB of the links in this group
  double computeGroupScale(const std::string &group);    
  void processInteractiveMarkerFeedback(const visualization_msgs::InteractiveMarkerFeedbackConstPtr& feedback);  
  
  planning_models::KinematicModelConstPtr kmodel_;
  
  std::vector<EndEffector> active_eef_;
  std::vector<VirtualJoint> active_vj_;
  
  std::map<std::string, InteractionHandlerPtr> handlers_;
  std::map<std::string, std::size_t> shown_markers_;
  
  interactive_markers::InteractiveMarkerServer *int_marker_server_;
};

typedef boost::shared_ptr<RobotInteraction> RobotInteractionPtr;
typedef boost::shared_ptr<const RobotInteraction> RobotInteractionConstPtr;

}

#endif
