#ifndef CNN_NEAT_NODE_H
#define CNN_NEAT_NODE_H

#include <fstream>
using std::ofstream;
using std::ifstream;

#include <iostream>
using std::cout;
using std::cerr;
using std::endl;
using std::ostream;
using std::istream;

#include <random>
using std::minstd_rand0;
using std::uniform_real_distribution;

#include <sstream>
using std::istringstream;

#include <string>
using std::string;

#include <vector>
using std::vector;


#include "image_tools/image_set.hxx"

#ifdef _MYSQL_
#include "common/db_conn.hxx"
#endif

#include "common/random.hxx"

#define RELU_MIN 0
#define RELU_MIN_LEAK 0.005

#define RELU_MAX 5.5
#define RELU_MAX_LEAK 0.00

#define INPUT_NODE 0
#define HIDDEN_NODE 1
#define OUTPUT_NODE 2
#define SOFTMAX_NODE 3


class CNN_Node {
    private:
        int node_id;
        int exact_id;
        int genome_id;

        int innovation_number;
        double depth;

        int size_x, size_y;

        //bias only applied to input of the node
        //output is after pooling

        vector< vector<double> > values;
        vector< vector<double> > errors;
        vector< vector<double> > gradients;
        vector< vector<double> > bias;
        vector< vector<double> > best_bias;
        vector< vector<double> > bias_velocity;
        vector< vector<double> > best_bias_velocity;

        int weight_count;

        int type;

        int total_inputs;
        int inputs_fired;

        bool visited;
        bool needs_initialization;

    public:
        CNN_Node();

        CNN_Node(int _innovation_number, double _depth, int _input_size_x, int _input_size_y, int type);

        CNN_Node* copy() const;

#ifdef _MYSQL_
        CNN_Node(int node_id);
        void export_to_database(int exact_id, int genome_id);

        int get_node_id() const;
#endif

        bool needs_init() const;
        int get_size_x() const;
        int get_size_y() const;

        void reset_weight_count();
        void add_weight_count(int _weight_count);
        int get_weight_count() const;

        int get_innovation_number() const;

        double get_depth() const;

        bool is_fixed() const;
        bool is_hidden() const;
        bool is_input() const;
        bool is_output() const;
        bool is_softmax() const;

        bool is_visited() const;
        void visit();
        void set_unvisited();

        void initialize_bias(minstd_rand0 &generator, NormalDistribution &normal_disribution);
        void reset_velocities();

        bool has_nan() const;
        bool has_zero_bias() const;
        bool has_zero_best_bias() const;
        void propagate_bias(double mu, double learning_rate, double weight_decay);

        void set_values(const Image &image, int channel, int rows, int cols, bool perform_dropout, minstd_rand0 &generator, double input_dropout_probability);
        double get_value(int y, int x);
        void set_value(int y, int x, double value);
        vector< vector<double> >& get_values();

        double get_error(int y, int x);
        void set_error(int y, int x, double value);
        vector< vector<double> >& get_errors();

        void set_gradient(int y, int x, double value);
        vector< vector<double> >& get_gradients();

        void print(ostream &out);

        void reset();

        void save_best_bias();
        void set_bias_to_best();

        void resize_arrays();
        bool modify_size_x(int change);
        bool modify_size_y(int change);

        void add_input();
        void disable_input();
        int get_number_inputs() const;
        int get_inputs_fired() const;
        void input_fired(bool perform_dropout, minstd_rand0 &generator, double hidden_dropout_probability);

        void print_statistics();

        friend ostream &operator<<(ostream &os, const CNN_Node* node);
        friend istream &operator>>(istream &is, CNN_Node* node);
};

/*
template<class T>
void parse_array_2d(T ***output, istringstream &iss, int size_x, int size_y);
*/

double read_hexfloat(istream &infile);
void write_hexfloat(ostream &outfile, double value);

struct sort_CNN_Nodes_by_depth {
    bool operator()(const CNN_Node *n1, const CNN_Node *n2) {
        if (n1->get_depth() < n2->get_depth()) {
            return true;
        } else if (n1->get_depth() == n2->get_depth()) {
            //make sure the order of the nodes is *always* the same
            //going through the nodes in different orders may change
            //the output of backpropagation
            if (n1->get_innovation_number() < n2->get_innovation_number()) {
                return true;
            } else {
                return false;
            }
        } else {
            return false;
        }
    }
};

#endif
