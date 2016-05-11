#include "object_tracking_test.hpp"
#include "correlation_trackers.hpp"

#include <dlib/dir_nav.h>

#include <opencv2/core/utility.hpp>
#include <opencv2/bgsegm.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/video/background_segm.hpp>
#include <opencv2/tracking.hpp>

#include <iostream>
#include <sstream>
#include <vector>

namespace{

void correlation_tracker_test(std::string const &folder,
                              size_t frame_num,
                              std::vector<cv::Rect> const &roi)
{
    auto files =
            dlib::get_files_in_directory_tree(folder,
                                              dlib::match_ending(".jpg"));
    std::sort(std::begin(files), std::end(files));

    auto img = cv::imread(folder + "/" +
                          files[frame_num].name());
    correlation_tracker tracker;
    for(size_t i = 0; i != roi.size(); ++i){
        tracker.add_track(img, roi[i]);
    }

    std::vector<cv::Scalar> const colors{{255, 0, 0},
                                         {0, 255, 0},
                                         {0, 0, 255},
                                         {255, 255, 0}};

    for(size_t i = frame_num + 1; i != files.size(); ++i){
        img = cv::imread(folder + "/" + files[i].name());
        if(!img.empty()){
            tracker.update(img);
            auto const positions = tracker.get_position();
            for(size_t j = 0; j != positions.size(); ++j){
                cv::rectangle(img, positions[j], colors[j]);
            }
            cv::imshow("tracking demo", img);
            int const key = cv::waitKey(10);
            if(key == 'q'){
                break;
            }
        }else{
            break;
        }
    }
}

}

void test_correlation_track()
{
    //<image file='v1_frames\img00433.jpg'>
    //    <box top='139' left='145' width='40' height='92'/>
    //    <box top='141' left='287' width='46' height='86'/>
    //</image>
    correlation_tracker_test("v1_frames", {433},
    {{145,139,40,92}, {287,141,46,86}});

    //<image file='v2_frames\img00398.jpg'>
    //    <box top='156' left='201' width='46' height='103'/>
    //    <box top='170' left='281' width='32' height='69'/>
    //</image>
    //correlation_tracker_test("v2_frames", {398},
    //{{201,156,46,103}, {281,170,32,69}});

    //<image file='v3_frames\img01713.jpg'>
    //    <box top='162' left='100' width='40' height='105'/>
    //    <box top='176' left='164' width='41' height='88'/>
    //    <box top='188' left='259' width='30' height='63'/>
    //    <box top='166' left='318' width='40' height='101'/>
    //</image>
    //correlation_tracker_test("v3_frames", {1713},
    //{{100,162,40,105}, {164,176,41,88},
    // {259,188,30,63}, {318,166,40,101}});
}

void test_gmg()
{
    using namespace cv::bgsegm;

    cv::Ptr<cv::BackgroundSubtractor> fgbg =
            createBackgroundSubtractorGMG(20, 0.7);
    cv::Ptr<cv::BackgroundSubtractor> mog2 =
            cv::createBackgroundSubtractorMOG2(500,16,false);
    if(!fgbg || !mog2){
        return;
    }

    cv::VideoCapture cap;
    cap.open("v1.mp4");
    if(!cap.isOpened()){
        std::cout<<"cannot read video"<<std::endl;
        return;
    }

    cv::Mat frame, fgmask, fgmask2;
    while(1){
        cap >> frame;
        if(frame.empty()){
            break;
        }
        fgbg->apply(frame, fgmask);
        mog2->apply(frame, fgmask2);
        //frame.convertTo(segm, CV_8U, 0.5);
        //segm = frame + {100,100,0}
        //cv::add(frame, cv::Scalar{100,100,0}, segm, fgmask);
        cv::imshow("frame", frame);
        cv::imshow("fg seg", fgmask);
        cv::imshow("mog2", fgmask2);
        int c = cv::waitKey(30);
        if (c == 'q' || c == 'Q' || (c & 255) == 27){
            break;
        }
    }
}

void test_tracking_module()
{
    cv::MultiTrackerTLD trackers;
    std::vector<cv::Rect2d> const objects
    {{145,139,40,92}, {287,141,46,86}};
    //std::vector<cv::Rect2d> objects;

    auto files =
            dlib::get_files_in_directory_tree("v1_frames",
                                              dlib::match_ending(".jpg"));
    std::sort(std::begin(files), std::end(files));

    auto frame = cv::imread("v1_frames/" +
                            files[433].name());
    //cv::selectROI("tracker", frame, objects);
    for(auto const &obj : objects){
        trackers.addTarget(frame, obj, "BOOSTING");
    }
    //trackers.add(frame, objects);

    for(size_t i = 434; i != files.size(); ++i){
        frame = cv::imread("v1_frames/" + files[i].name());
        if(!frame.empty()){
            //trackers.update(frame, objects);
            auto const track_all = trackers.update(frame);
            if(!track_all){
                std::cout<<"maybe losing object"<<std::endl;
                std::cin.get();
            }
            for(size_t j = 0; j < trackers.targetNum; ++j){
                //std::cout<<j<<" : "<<objects[j]<<std::endl;
                //cv::rectangle(frame, objects[j], colors[j]);
                cv::rectangle(frame, trackers.boundingBoxes[j],
                              trackers.colors[j]);
            }
            cv::imshow("tracker", frame);
            int const key = cv::waitKey(30);
            if(key == 'q'){
                break;
            }
        }else{
            break;
        }
    }
}