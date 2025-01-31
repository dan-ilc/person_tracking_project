/*this file has a class definition:
*  - public
*      - constructor tracker(nh, kf_parmas);
*      - load_kf() {}
*      - updateKf() { updates kalman filter) and other get methods
*  - private has node handle
*     - THIS ENABLES DIFFERENT THINGS TO TALK TO EACH OTHER.
*     - void callback(msg);
*     - kf;
*/
#include <ros/ros.h> // because this is a robot
#include <ros/console.h>
#include <sensor_msgs/PointCloud2.h>
#include <vector>
///////////////////

// sensor msgs includes specific includes
#include <sensor_msgs/PointCloud2.h>
#include <sensor_msgs/PointCloud.h>
#include <sensor_msgs/point_cloud_conversion.h>

#include <Eigen/Dense>

// TF includes
#include <tf/transform_listener.h>
#include <tf/transform_broadcaster.h>

#include <pcl/common/centroid.h>
#include <pcl_ros/transforms.h>

// PCL includes
#include <pcl/filters/passthrough.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/io/pcd_io.h>

// includes for cluster extraction
#include <pcl_conversions/pcl_conversions.h>
#include <pcl/common/transforms.h> //allows us to use pcl::transformPointCloud function.
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/filters/extract_indices.h>
#include <pcl/features/normal_3d.h>
#include <pcl/kdtree/kdtree.h>
#include <pcl/sample_consensus/method_types.h>
#include <pcl/sample_consensus/model_types.h>
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/segmentation/extract_clusters.h>
#include <visualization_msgs/Marker.h> // for the marker


#include <stdlib.h> // for random
#include <cmath> // used for euclidean norm
////////////////////////////////


//using namespace Eigen;
using namespace std;

#include "kalman_filter.h"
#ifndef tracker_H
#define tracker_H



class Tracker {

public:
    Tracker(ros::NodeHandle nh,
            int max_cluster_size,
            int min_cluster_size,
            double cluster_tolerance,
            double seg_dist_threshold,
            bool verbose,
            bool publishing); // initiate a constructor with a nodehandle and parameters for the kalman filter

    void setupKalmanFilter(VectorXd x0,
                           double dt,
                           const Eigen::MatrixXd& A,
                           const Eigen::MatrixXd& C,
                           const Eigen::MatrixXd& Q,
                           const Eigen::MatrixXd& R,
                           const Eigen::MatrixXd& P); // call the constructor of and initiate the kalman filter with params x0
    void updateKf(VectorXd y); // update the kalman filter with a new estimate y

private:
    ////// General Pointcloud Methods
    void callback(const sensor_msgs::PointCloud2ConstPtr &cloud_msg); // this method is called whenever the Tracker sees a new pointcloud
    void applyPassthroughFilter(const sensor_msgs::PointCloud2ConstPtr input_cloud, sensor_msgs::PointCloud2 &output_cloud); // filters points outside of a defined cube
    void removeOutOfPlanePoints(pcl::PointCloud<pcl::PointXYZRGB>::Ptr &cloud_ptr); // remove non-planar-inlying points
    void applyBaseOdomTransformation(sensor_msgs::PointCloud2 input_cloud, sensor_msgs::PointCloud2 &output_cloud); // transforms the cloud into the odom frame
    void convertSM2ToPclPtr(sensor_msgs::PointCloud2 input_cloud, pcl::PointCloud<pcl::PointXYZRGB>::Ptr &output_ptr);
    void applyVoxelGrid(pcl::PointCloud<pcl::PointXYZRGB>::Ptr &cloud_ptr);
    void splitCloudPtrIntoClusters(pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_ptr, vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> &cloud_cluster_vector);
    void assignRandomColour(pcl::PointCloud<pcl::PointXYZRGB>::Ptr &input_cloud);
    vector<pcl::PointIndices> getClusterIndices(pcl::PointCloud<pcl::PointXYZRGB>::Ptr cloud_ptr);

    ////// Centroid Pointcloud Methods
    void getCentroidsOfClusters (vector<pcl::PointCloud<pcl::PointXYZRGB>::Ptr> cloud_cluster_vector, vector<Eigen::VectorXd> &centroid_coord_array);
    void getClusterCentroid(pcl::PointCloud<pcl::PointXYZRGB>::Ptr cluster_ptr, VectorXd &coord_centroid); //returns a vector of centroid coordinates
    void processCentroidCoords(vector<VectorXd> centroid_coord_array);

    ////// I/O Methods
    void initialiseSubscribersAndPublishers(); // initialises all the subscribers and publishers
    void publishMarker(VectorXd x_hat,double scale_x,double scale_y,double scale_z);
    void publishTransform(VectorXd coordinates, string target_frame_id); // publishes a transform at the 3D coordinate Vector coordinates

    ////// Kalman Filter Methods
    KalmanFilter kf_; // our private copy of a kalman filter
    VectorXd getState(); // returns the current position estimate

    ////// I/O Variables
    ros::Subscriber point_cloud_sub_; // private copy of subscriber for velodyne
    tf::TransformBroadcaster br_; // to broadcast tfs on
    ros::NodeHandle nh_; // the nodehandle
    ros::Publisher pub_raw_; // raw input cloud
    ros::Publisher pub_trans_; // transformed cloud
    ros::Publisher pub_zfilt_; // passthrough filtered cloud
    ros::Publisher pub_ds_; // downsampled cloud
    ros::Publisher pub_seg_filter_; // downsampled cloud
    ros::Publisher pub_centroid_; // centroid cluster
    ros::Publisher pub_marker_; // for markers
    boost::shared_ptr<tf::TransformListener> odom_base_ls_; // setup the transform listener so we can listen to the odom_base_tf
    bool verbose_, publishing_; // do we print? do we publish pointclouds>?


    ////// Clustering parameters
    int max_cluster_size_, min_cluster_size_; // clustering parameters
    double cluster_tolerance_,seg_dist_threshold_; // seg_dist is how close a point must be to the model in order to be considered an inlier:

};
#endif // tracker_H
