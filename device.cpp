#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <cmath>

using namespace std;

int streamcounter; ///< Global variable to keep track of stream creation.

const int MIXER_OUTPUTS = 1;
const float POSSIBLE_ERROR = 0.01;

/**
 * @class Stream
 * @brief Represents a chemical stream with a name and mass flow.
 */
class Stream
{
private:
    double mass_flow; ///< The mass flow rate of the stream.
    string name;      ///< The name of the stream.

public:
    /**
     * @brief Constructor to create a Stream with a unique name.
     * @param s An integer used to generate a unique name for the stream.
     */
    Stream(int s){setName("s"+std::to_string(s));}

     /**
     * @brief Set the name of the stream.
     * @param s The new name for the stream.
     */
    void setName(string s){name=s;}

    /**
     * @brief Get the name of the stream.
     * @return The name of the stream.
     */
    string getName(){return name;}

    /**
     * @brief Set the mass flow rate of the stream.
     * @param m The new mass flow rate value.
     */
    void setMassFlow(double m){mass_flow=m;}

    /**
     * @brief Get the mass flow rate of the stream.
     * @return The mass flow rate of the stream.
     */
    double getMassFlow() const {return mass_flow;}

    /**
     * @brief Print information about the stream.
     */
    void print() { cout << "Stream " << getName() << " flow = " << getMassFlow() << endl; }
};

/**
 * @class Device
 * @brief Represents a device that manipulates chemical streams.
 */
class Device
{
protected:
    vector<shared_ptr<Stream>> inputs;  ///< Input streams connected to the device.
    vector<shared_ptr<Stream>> outputs; ///< Output streams produced by the device.
    int inputAmount;
    int outputAmount;
    
public:
    Device() : inputAmount(0), outputAmount(0) {}
    const vector<shared_ptr<Stream>>& getInputs() const { return inputs; }
    const vector<shared_ptr<Stream>>& getOutputs() const { return outputs; }
    
      /**
     * @brief Add an input stream to the device.
     * @param s A shared pointer to the input stream.
     */
    virtual void addInput(shared_ptr<Stream> s){
        if(inputs.size() < inputAmount) inputs.push_back(s);
        else throw "INPUT STREAM LIMIT!";
    }
    
      /**
     * @brief Add an output stream to the device.
     * @param s A shared pointer to the output stream.
     */
    virtual void addOutput(shared_ptr<Stream> s){
        if(outputs.size() < outputAmount) outputs.push_back(s);
        else throw "OUTPUT STREAM LIMIT!";
    }

      /**
     * @brief Update the output streams of the device (to be implemented by derived classes).
     */
    virtual void updateOutputs() = 0;
};

class Absorber : public Device
{
public:
    Absorber() {
        inputAmount = 2; 
        outputAmount = 2; 
    }
    
    void addInput(shared_ptr<Stream> s) override {
        if (inputs.size() >= inputAmount) {
            throw "Too much inputs for absorber";
        }
        inputs.push_back(s);
    }
    
    void addOutput(shared_ptr<Stream> s) override {
        if (outputs.size() >= outputAmount) {
            throw "Too much outputs for absorber";
        }
        outputs.push_back(s);
    }
    
    void updateOutputs() override {
        if (inputs.size() < inputAmount || outputs.size() < outputAmount) {
            throw "Absorber requires exactly 2 inputs and 2 outputs";
        }
        
        double input1_mass = inputs[0]->getMassFlow();
        double input2_mass = inputs[1]->getMassFlow();
        
        double total_mass = input1_mass + input2_mass;
        outputs[0]->setMassFlow(total_mass * 0.3); 
        outputs[1]->setMassFlow(total_mass * 0.7); 
    }
};

void testTooManyInputs()
{
    cout << "\nTest 1: Too Many Input Streams" << endl;
    
    streamcounter = 0;
    Absorber absorber;
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));

    shared_ptr<Stream> s3(new Stream(++streamcounter));
    
    absorber.addInput(s1);
    absorber.addInput(s2);
    
    try {
        absorber.addInput(s3);
        cout << "Test 1 failed" << endl;
    } 
    catch(const char* ex) {
        if (string(ex) == "Too much inputs for absorber")
            cout << "Test 1 passed" << endl;
        else
            cout << "Test 1 failed: " << ex << endl;
    }
}

void testTooManyOutputs()
{
    cout << "\nTest 2: Too Many Output Streams" << endl;
    
    streamcounter = 0;
    Absorber absorber;
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));

    shared_ptr<Stream> s3(new Stream(++streamcounter));
    
    absorber.addOutput(s1);
    absorber.addOutput(s2);
    
    try {
        absorber.addOutput(s3);
        cout << "Test 2 failed" << endl;
    } catch(const char* ex) {
        if (string(ex) == "Too much outputs for absorber")
            cout << "Test 2 passed" << endl;
        else
            cout << "Test 2 failed: " << ex << endl;
    }
}

void testSetOutputs(){
    cout << "\nTest 3: Input-Output Mass Balance" << endl;

    streamcounter=0;
    Absorber absorber;
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    shared_ptr<Stream> s4(new Stream(++streamcounter));

    s1->setMassFlow(10.0);
    s2->setMassFlow(15.0);

    absorber.addInput(s1);
    absorber.addInput(s2);
    absorber.addOutput(s3);
    absorber.addOutput(s4);

    absorber.updateOutputs();
    
    double totalOutput = absorber.getOutputs()[0]->getMassFlow() + absorber.getOutputs()[1]->getMassFlow();
    double totalinput = absorber.getInputs()[0]->getMassFlow() + absorber.getInputs()[1]->getMassFlow();
    
    if(abs(totalOutput - totalinput) < POSSIBLE_ERROR) {
        cout << "Test 3 passed" << endl;
    } else {
        cout << "Test 3 failed: " << totalOutput << " != " <<totalinput << endl;
    }
}

void testOutputDistribution()
{
    cout << "\nTest 4: Output Mass Distribution (30%/70%)" << endl;
    
    streamcounter = 0;
    Absorber absorber;
    
    shared_ptr<Stream> s1(new Stream(++streamcounter));
    shared_ptr<Stream> s2(new Stream(++streamcounter));
    shared_ptr<Stream> s3(new Stream(++streamcounter));
    shared_ptr<Stream> s4(new Stream(++streamcounter));

    s1->setMassFlow(60.0);
    s2->setMassFlow(40.0);

    absorber.addInput(s1);
    absorber.addInput(s2);
    absorber.addOutput(s3);
    absorber.addOutput(s4);

    absorber.updateOutputs();
    
    double total_input = 60.0 + 40.0;
    double expected_output1 = total_input * 0.3;
    double expected_output2 = total_input * 0.7;
    
    double actual_output1 = absorber.getOutputs()[0]->getMassFlow();
    double actual_output2 = absorber.getOutputs()[1]->getMassFlow();
    
    bool test1_passed = abs(actual_output1 - expected_output1) < POSSIBLE_ERROR;
    bool test2_passed = abs(actual_output2 - expected_output2) < POSSIBLE_ERROR;
    
    if (test1_passed && test2_passed) {
        cout << "Test 4 passed" << endl;
    } else {
        cout << "Test 4 failed: Incorrect distribution" << endl;
    }
}
void tests()
{
    testTooManyInputs();
    testTooManyOutputs();
    testSetOutputs();
    testOutputDistribution();
}

int main()
{
    streamcounter = 0;
    tests();
    return 0;
}