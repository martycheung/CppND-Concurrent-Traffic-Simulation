#include <iostream>
#include <random>
#include <chrono>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */

template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function. 
    std::unique_lock<std::mutex> lck(mtx);
    // Wait for message
    _condition.wait(lck, [this] { return !_queue.empty(); });

    // Get last message in queue using move semantics
    T msg = std::move(_queue.back());
    // remove last message from queue
    _queue.pop_back();

    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.
    
    // Put lock on
    std::lock_guard<std::mutex> lck(mtx);

    // add message to queue
    _queue.push_back(std::move(msg));
    _condition.notify_one(); 
}


/* Implementation of class "TrafficLight" */

TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while (true) {

        // Sleep for 1ms between cycles to reduce cpu load
        std::this_thread::sleep_for(std::chrono::milliseconds(1));

        TrafficLightPhase received_msg = this->_trafficMessage.receive();

        if(received_msg == TrafficLightPhase::green) {
            return;
        }

        
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called. To do this, use the thread queue in the base class.
    // TrafficLight inherits from TrafficObject - each trafficobject has a thread queue
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));

}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles. 

    // Generate random float between 4 and 6
    std::random_device rd;     // only used once to initialise (seed) engine
    std::mt19937 eng(rd());    // random-number engine used (Mersenne-Twister in this case)
    std::uniform_real_distribution<> dist(4, 6);

    //Get random cycle time
    auto cycle_time = dist(eng);

    // Start timer
    auto start = std::chrono::high_resolution_clock::now();

    while (true) {
        // Sleep for 1ms between cycles
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        // Get current duration
        std::chrono::duration<double> duration = std::chrono::high_resolution_clock::now() - start;

        if (duration.count() >= cycle_time) {

            // toggle the traffic light phase
            _currentPhase = (_currentPhase == TrafficLightPhase::green ? TrafficLightPhase::red : TrafficLightPhase::green);
            
            // Send message to queue
            _trafficMessage.send(std::move(_currentPhase));

            // Reset Clock and generate another cycle time
            start = std::chrono::high_resolution_clock::now();
            cycle_time = dist(eng);

        }
    }
}

