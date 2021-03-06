#include <iomanip>
using std::setw;

#include <iostream>
using std::cerr;
using std::cout;
using std::endl;

#include <random>
using std::minstd_rand0;

#include <string>
using std::string;

#include <vector>
using std::vector;

#include "common/arguments.hxx"

#include "strategy/exact.hxx"
#include "strategy/cnn_genome.hxx"
#include "strategy/cnn_edge.hxx"
#include "strategy/cnn_node.hxx"

#include "image_tools/image_set.hxx"

int main(int argc, char **argv) {
    vector<string> arguments = vector<string>(argv, argv + argc);

    string binary_training_filename;
    get_argument(arguments, "--training_file", true, binary_training_filename);

    string binary_testing_filename;
    get_argument(arguments, "--testing_file", true, binary_testing_filename);

    int max_epochs;
    get_argument(arguments, "--max_epochs", true, max_epochs);

    bool reset_edges;
    get_argument(arguments, "--reset_edges", true, reset_edges);

    double learning_rate;
    get_argument(arguments, "--learning_rate", true, learning_rate);

    double learning_rate_decay;
    get_argument(arguments, "--learning_rate_decay", true, learning_rate_decay);

    double weight_decay;
    get_argument(arguments, "--weight_decay", true, weight_decay);

    double weight_decay_decay;
    get_argument(arguments, "--weight_decay_decay", true, weight_decay_decay);

    double mu;
    get_argument(arguments, "--mu", true, mu);

    double mu_decay;
    get_argument(arguments, "--mu_decay", true, mu_decay);

    double input_dropout_probability;
    get_argument(arguments, "--input_dropout_probability", true, input_dropout_probability);

    double hidden_dropout_probability;
    get_argument(arguments, "--hidden_dropout_probability", true, hidden_dropout_probability);

    int velocity_reset;
    get_argument(arguments, "--velocity_reset", true, velocity_reset);


    Images training_images(binary_training_filename);
    Images testing_images(binary_testing_filename, training_images.get_average(), training_images.get_std_dev());

    int node_innovation_count = 0;
    int edge_innovation_count = 0;
    vector<CNN_Node*> nodes;
    vector<CNN_Node*> layer1_nodes;
    vector<CNN_Node*> softmax_nodes;

    vector<CNN_Edge*> edges;

    minstd_rand0 generator(time(NULL));
    NormalDistribution normal_distribution;

    CNN_Node *input_node = new CNN_Node(node_innovation_count, 0, training_images.get_image_rows(), training_images.get_image_cols(), INPUT_NODE);
    node_innovation_count++;
    nodes.push_back(input_node);

    //first layer of filters
    //input node goes to 5 filters
    for (int32_t i = 0; i < 10; i++) {
        CNN_Node *layer1_node = new CNN_Node(++node_innovation_count, 1, 5, 5, HIDDEN_NODE);
        nodes.push_back(layer1_node);
        layer1_nodes.push_back(layer1_node);

        edges.push_back( new CNN_Edge(input_node, layer1_node, false, ++edge_innovation_count) );
    }

    for (int32_t i = 0; i < training_images.get_number_classes(); i++) {
        CNN_Node *softmax_node = new CNN_Node(++node_innovation_count, 2, 1, 1, SOFTMAX_NODE);
        nodes.push_back(softmax_node);
        softmax_nodes.push_back(softmax_node);

        for (int32_t j = 0; j < 10; j++) {
            edges.push_back( new CNN_Edge(layer1_nodes[j], softmax_node, false, ++edge_innovation_count) );

        }
    }

    long genome_seed = generator();
    cout << "seeding genome with: " << genome_seed << endl;

    CNN_Genome *genome = new CNN_Genome(1, genome_seed, max_epochs, reset_edges, velocity_reset, mu, mu_decay, learning_rate, learning_rate_decay, weight_decay, weight_decay_decay, input_dropout_probability, hidden_dropout_probability, nodes, edges);
    //save the weights and bias of the initially generated genome for reuse
    genome->initialize();

    cout << "number edges: " << edges.size() << ", total weights: " << genome->get_number_weights() << ", total biases: " << genome->get_number_biases() << endl;

    ofstream outfile("two_layer.gv");
    genome->print_graphviz(outfile);
    outfile.close();

    genome->stochastic_backpropagation(training_images);

    cout << "evaluating best weights on testing data." << endl;
    genome->set_to_best();

    double error;
    int predictions;
    genome->evaluate(testing_images, error, predictions);
}
