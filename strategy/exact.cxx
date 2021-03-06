#include <algorithm>
using std::sort;
using std::upper_bound;

#include <chrono>

#include <iomanip>
using std::fixed;
using std::setprecision;
using std::setw;
using std::left;
using std::right;

#include <iostream>
using std::fstream;
using std::ostream;
using std::istream;

#include <limits>
using std::numeric_limits;

#include <random>
using std::minstd_rand0;
using std::uniform_int_distribution;
using std::uniform_real_distribution;

#include <sstream>
using std::istringstream;
using std::ostringstream;

#include <string>
using std::to_string;

#include <vector>
using std::vector;

#include "image_tools/image_set.hxx"
#include "cnn_node.hxx"
#include "cnn_edge.hxx"
#include "cnn_genome.hxx"
#include "exact.hxx"

#ifdef _MYSQL_
#include "common/db_conn.hxx"
#endif

#include "stdlib.h"

#ifdef _MYSQL_

bool EXACT::exists_in_database(int exact_id) {
    ostringstream query;

    query << "SELECT * FROM exact_search WHERE id = " << exact_id;

    mysql_exact_query(query.str());
    
    MYSQL_RES *result = mysql_store_result(exact_db_conn);
    bool found;
    if (result != NULL) {
        if (mysql_num_rows(result) > 0) {
            found = true;
        } else {
            found = false;
        }

    } else {
        cerr << "ERROR in mysql query: '" << query.str() << "'" << endl;
        exit(1);
    }
    mysql_free_result(result);

    return found;
}

EXACT::EXACT(int exact_id) {
    ostringstream query;

    query << "SELECT * FROM exact_search WHERE id = " << exact_id;

    mysql_exact_query(query.str());
    
    MYSQL_RES *result = mysql_store_result(exact_db_conn);
    if (result != NULL) {
        MYSQL_ROW row = mysql_fetch_row(result);

        id = exact_id;  //is row 0

        int column = 0;

        search_name = string(row[++column]);
        output_directory = string(row[++column]);
        samples_filename = string(row[++column]);

        number_images = atoi(row[++column]);
        image_channels = atoi(row[++column]);
        image_rows = atoi(row[++column]);
        image_cols = atoi(row[++column]);
        number_classes = atoi(row[++column]);

        population_size = atoi(row[++column]);
        node_innovation_count = atoi(row[++column]);
        edge_innovation_count = atoi(row[++column]);

        genomes_generated = atoi(row[++column]);
        inserted_genomes = atoi(row[++column]);
        max_genomes = atoi(row[++column]);

        reset_weights = atoi(row[++column]);
        max_epochs = atoi(row[++column]);

        initial_mu_min = atof(row[++column]);
        initial_mu_max = atof(row[++column]);
        mu_min = atof(row[++column]);
        mu_max = atof(row[++column]);

        initial_mu_delta_min = atof(row[++column]);
        initial_mu_delta_max = atof(row[++column]);
        mu_delta_min = atof(row[++column]);
        mu_delta_max = atof(row[++column]);

        initial_learning_rate_min = atof(row[++column]);
        initial_learning_rate_max = atof(row[++column]);
        learning_rate_min = atof(row[++column]);
        learning_rate_max = atof(row[++column]);

        initial_learning_rate_delta_min = atof(row[++column]);
        initial_learning_rate_delta_max = atof(row[++column]);
        learning_rate_delta_min = atof(row[++column]);
        learning_rate_delta_max = atof(row[++column]);

        initial_weight_decay_min = atof(row[++column]);
        initial_weight_decay_max = atof(row[++column]);
        weight_decay_min = atof(row[++column]);
        weight_decay_max = atof(row[++column]);

        initial_weight_decay_delta_min = atof(row[++column]);
        initial_weight_decay_delta_max = atof(row[++column]);
        weight_decay_delta_min = atof(row[++column]);
        weight_decay_delta_max = atof(row[++column]);

        initial_input_dropout_probability_min = atof(row[++column]);
        initial_input_dropout_probability_max = atof(row[++column]);
        input_dropout_probability_min = atof(row[++column]);
        input_dropout_probability_max = atof(row[++column]);

        initial_hidden_dropout_probability_min = atof(row[++column]);
        initial_hidden_dropout_probability_max = atof(row[++column]);
        hidden_dropout_probability_min = atof(row[++column]);
        hidden_dropout_probability_max = atof(row[++column]);

        initial_velocity_reset_min = atoi(row[++column]);
        initial_velocity_reset_max = atoi(row[++column]);
        velocity_reset_min = atoi(row[++column]);
        velocity_reset_max = atoi(row[++column]);

        sort_by_fitness = atoi(row[++column]);
        reset_weights_chance = atof(row[++column]);

        crossover_rate = atof(row[++column]);
        more_fit_parent_crossover = atof(row[++column]);
        less_fit_parent_crossover = atof(row[++column]);

        number_mutations = atoi(row[++column]);
        edge_disable = atof(row[++column]);
        edge_enable = atof(row[++column]);
        edge_split = atof(row[++column]);
        edge_add = atof(row[++column]);
        edge_change_stride = atof(row[++column]);
        node_change_size = atof(row[++column]);
        node_change_size_x = atof(row[++column]);
        node_change_size_y = atof(row[++column]);
        node_change_pool_size = atof(row[++column]);

        istringstream generator_iss(row[++column]);
        generator_iss >> generator;
        //cout << "read generator from database: " << generator << endl;

        istringstream normal_distribution_iss(row[++column]);
        normal_distribution_iss >> normal_distribution;
        //cout << "read normal_distribution from database: " << normal_distribution << endl;

        istringstream rng_long_iss(row[++column]);
        rng_long_iss >> rng_long;
        //cout << "read rng_long from database: " << rng_long << endl;

        istringstream rng_double_iss(row[++column]);
        rng_double_iss >> rng_double;
        //cout << "read rng_double from database: " << rng_double << endl;

        inserted_from_disable_edge = atoi(row[++column]);
        inserted_from_enable_edge = atoi(row[++column]);
        inserted_from_split_edge = atoi(row[++column]);
        inserted_from_add_edge = atoi(row[++column]);
        inserted_from_change_size = atoi(row[++column]);
        inserted_from_change_size_x = atoi(row[++column]);
        inserted_from_change_size_y = atoi(row[++column]);
        inserted_from_crossover = atoi(row[++column]);
        inserted_from_reset_weights = atoi(row[++column]);

        generated_from_disable_edge = atoi(row[++column]);
        generated_from_enable_edge = atoi(row[++column]);
        generated_from_split_edge = atoi(row[++column]);
        generated_from_add_edge = atoi(row[++column]);
        generated_from_change_size = atoi(row[++column]);
        generated_from_change_size_x = atoi(row[++column]);
        generated_from_change_size_y = atoi(row[++column]);
        generated_from_crossover = atoi(row[++column]);
        generated_from_reset_weights = atoi(row[++column]);

        ostringstream genome_query;
        genome_query << "SELECT id FROM cnn_genome WHERE exact_id = " << id << " ORDER BY best_error LIMIT " << population_size;
        //cout << genome_query.str() << endl;

        mysql_exact_query(genome_query.str());

        MYSQL_RES *genome_result = mysql_store_result(exact_db_conn);

        //cout << "got genome result" << endl;

        MYSQL_ROW genome_row;
        while ((genome_row = mysql_fetch_row(genome_result)) != NULL) {
            int genome_id = atoi(genome_row[0]);
            //cout << "got genome with id: " << genome_id << endl;

            CNN_Genome *genome = new CNN_Genome(genome_id);
            genomes.push_back(genome);
        }

        cout << "got " << genomes.size() << " genomes." << endl;
        cout << "population_size: " << population_size << endl;
        cout << "inserted_genomes: " << inserted_genomes << ", max_genomes: " << max_genomes << endl;

        ostringstream node_query;
        node_query << "SELECT id FROM cnn_node WHERE exact_id = " << id << " AND genome_id = 0";
        //cout << node_query.str() << endl;

        mysql_exact_query(node_query.str());

        MYSQL_RES *node_result = mysql_store_result(exact_db_conn);

        //cout << "got node result!" << endl;

        MYSQL_ROW node_row;
        while ((node_row = mysql_fetch_row(node_result)) != NULL) {
            int node_id = atoi(node_row[0]);
            //cout << "got node with id: " << node_id << endl;

            CNN_Node *node = new CNN_Node(node_id);
            all_nodes.push_back(node);
        }

        //cout << "got all nodes!" << endl;
        mysql_free_result(node_result);

        ostringstream edge_query;
        edge_query << "SELECT id FROM cnn_edge WHERE exact_id = " << id << " AND genome_id = 0";
        //cout << edge_query.str() << endl;

        mysql_exact_query(edge_query.str());
        //cout << "edge query was successful!" << endl;

        MYSQL_RES *edge_result = mysql_store_result(exact_db_conn);
        //cout << "got edge result!" << endl;

        MYSQL_ROW edge_row;
        while ((edge_row = mysql_fetch_row(edge_result)) != NULL) {
            int edge_id = atoi(edge_row[0]);
            //cout << "got edge with id: " << edge_id << endl;

            CNN_Edge *edge = new CNN_Edge(edge_id);
            all_edges.push_back(edge);

            edge->set_nodes(all_nodes);
        }

        //cout << "got all edges!" << endl;
        mysql_free_result(edge_result);

        mysql_free_result(result);
    } else {
        cerr << "ERROR! could not find exact_search in database with id: " << exact_id << endl;
        exit(1);
    }
}

void EXACT::export_to_database() {
    ostringstream query;
    if (id >= 0) {
        query << "REPLACE INTO exact_search SET id = " << id << ",";
    } else {
        query << "INSERT INTO exact_search SET";
    }

    //cout << "exporting exact to database!" << endl;

    query << " search_name = '" << search_name << "'"
        << ", output_directory = '" << output_directory << "'"
        << ", samples_filename = '" << samples_filename << "'"

        << ", number_images = " << number_images
        << ", image_channels = " << image_channels
        << ", image_rows = " << image_rows
        << ", image_cols = " << image_cols
        << ", number_classes = " << number_classes

        << ", population_size = " << population_size
        << ", node_innovation_count = " << node_innovation_count
        << ", edge_innovation_count = " << edge_innovation_count

        << ", genomes_generated = " << genomes_generated
        << ", inserted_genomes = " << inserted_genomes
        << ", max_genomes = " << max_genomes

        << ", reset_weights = " << reset_weights
        << ", max_epochs = " << max_epochs

        << ", initial_mu_min = " << initial_mu_min
        << ", initial_mu_max = " << initial_mu_max
        << ", mu_min = " << mu_min
        << ", mu_max = " << mu_max

        << ", initial_mu_delta_min = " << initial_mu_delta_min
        << ", initial_mu_delta_max = " << initial_mu_delta_max
        << ", mu_delta_min = " << mu_delta_min
        << ", mu_delta_max = " << mu_delta_max

        << ", initial_learning_rate_min = " << initial_learning_rate_min
        << ", initial_learning_rate_max = " << initial_learning_rate_max
        << ", learning_rate_min = " << learning_rate_min
        << ", learning_rate_max = " << learning_rate_max

        << ", initial_learning_rate_delta_min = " << initial_learning_rate_delta_min
        << ", initial_learning_rate_delta_max = " << initial_learning_rate_delta_max
        << ", learning_rate_delta_min = " << learning_rate_delta_min
        << ", learning_rate_delta_max = " << learning_rate_delta_max

        << ", initial_weight_decay_min = " << initial_weight_decay_min
        << ", initial_weight_decay_max = " << initial_weight_decay_max
        << ", weight_decay_min = " << weight_decay_min
        << ", weight_decay_max = " << weight_decay_max

        << ", initial_weight_decay_delta_min = " << initial_weight_decay_delta_min
        << ", initial_weight_decay_delta_max = " << initial_weight_decay_delta_max
        << ", weight_decay_delta_min = " << weight_decay_delta_min
        << ", weight_decay_delta_max = " << weight_decay_delta_max

        << ", initial_input_dropout_probability_min = " << initial_input_dropout_probability_min
        << ", initial_input_dropout_probability_max = " << initial_input_dropout_probability_max
        << ", input_dropout_probability_min = " << input_dropout_probability_min
        << ", input_dropout_probability_max = " << input_dropout_probability_max

        << ", initial_hidden_dropout_probability_min = " << initial_hidden_dropout_probability_min
        << ", initial_hidden_dropout_probability_max = " << initial_hidden_dropout_probability_max
        << ", hidden_dropout_probability_min = " << hidden_dropout_probability_min
        << ", hidden_dropout_probability_max = " << hidden_dropout_probability_max

        << ", initial_velocity_reset_min = " << initial_velocity_reset_min
        << ", initial_velocity_reset_max = " << initial_velocity_reset_max
        << ", velocity_reset_min = " << velocity_reset_min
        << ", velocity_reset_max = " << velocity_reset_max

        << ", sort_by_fitness = " << sort_by_fitness
        << ", reset_weights_chance = " << reset_weights_chance

        << ", crossover_rate = " << crossover_rate
        << ", more_fit_parent_crossover = " << more_fit_parent_crossover
        << ", less_fit_parent_crossover = " << less_fit_parent_crossover

        << ", number_mutations = " << number_mutations
        << ", edge_disable = " << edge_disable
        << ", edge_enable = " << edge_enable
        << ", edge_split = " << edge_split
        << ", edge_add = " << edge_add
        << ", edge_change_stride = " << edge_change_stride
        << ", node_change_size = " << node_change_size
        << ", node_change_size_x = " << node_change_size_x
        << ", node_change_size_y = " << node_change_size_y
        << ", node_change_pool_size = " << node_change_pool_size

        << ", generator = '" << generator << "'"
        << ", normal_distribution = '" << normal_distribution << "'"
        << ", rng_long = '" << rng_long << "'"
        << ", rng_double = '" << rng_double << "'"

        << ", inserted_from_disable_edge = " << inserted_from_disable_edge
        << ", inserted_from_enable_edge = " << inserted_from_enable_edge
        << ", inserted_from_split_edge = " << inserted_from_split_edge
        << ", inserted_from_add_edge = " << inserted_from_add_edge
        << ", inserted_from_change_size = " << inserted_from_change_size
        << ", inserted_from_change_size_x = " << inserted_from_change_size_x
        << ", inserted_from_change_size_y = " << inserted_from_change_size_y
        << ", inserted_from_crossover = " << inserted_from_crossover
        << ", inserted_from_reset_weights = " << inserted_from_reset_weights

        << ", generated_from_disable_edge = " << generated_from_disable_edge
        << ", generated_from_enable_edge = " << generated_from_enable_edge
        << ", generated_from_split_edge = " << generated_from_split_edge
        << ", generated_from_add_edge = " << generated_from_add_edge
        << ", generated_from_change_size = " << generated_from_change_size
        << ", generated_from_change_size_x = " << generated_from_change_size_x
        << ", generated_from_change_size_y = " << generated_from_change_size_y
        << ", generated_from_crossover = " << generated_from_crossover
        << ", generated_from_reset_weights = " << generated_from_reset_weights;

    cout << query.str() << endl;
    mysql_exact_query(query.str());

    if (id < 0) {
        id = mysql_exact_last_insert_id();
        cout << "inserted EXACT search with id: " << id << endl;
    }

    //need to insert genomes
    for (uint32_t i = 0; i < genomes.size(); i++) {
        genomes[i]->export_to_database(id);
    }

    //need to insert all_nodes and all_edges
    //a genome id of 0 means that they are not assigned to
    //a particular genome
    for (uint32_t i = 0; i < all_nodes.size(); i++) {
        all_nodes.at(i)->export_to_database(id, 0);
    }

    for (uint32_t i = 0; i < all_edges.size(); i++) {
        all_edges.at(i)->export_to_database(id, 0);
    }

    if ((int32_t)genomes.size() == population_size) {
        if (sort_by_fitness) {
            double worst_fitness = genomes.back()->get_fitness();
            ostringstream delete_query;
            delete_query << "DELETE FROM cnn_genome WHERE exact_id = " << id << " AND best_error > " << worst_fitness;
            cout << delete_query.str() << endl;
            mysql_exact_query(delete_query.str());
        } else {
            double worst_predictions = genomes.back()->get_best_predictions();
            ostringstream delete_query;
            delete_query << "DELETE FROM cnn_genome WHERE exact_id = " << id << " AND best_predictions < " << worst_predictions;
            cout << delete_query.str() << endl;
            mysql_exact_query(delete_query.str());
        }

        ostringstream delete_node_query;
        delete_node_query << "DELETE FROM cnn_node WHERE exact_id = " << id << " AND genome_id > 0 AND NOT EXISTS(SELECT id FROM cnn_genome WHERE cnn_genome.id = cnn_node.genome_id)";
        cout <<  delete_node_query.str() << endl;
        mysql_exact_query(delete_node_query.str());

        ostringstream delete_edge_query;
        delete_edge_query << "DELETE FROM cnn_edge WHERE exact_id = " << id << " AND genome_id > 0 AND NOT EXISTS(SELECT id FROM cnn_genome WHERE cnn_genome.id = cnn_edge.genome_id)";
        cout <<  delete_edge_query.str() << endl;
        mysql_exact_query(delete_edge_query.str());
    }
}

void EXACT::update_database() {
    if (id < 0) {
        cerr << "ERROR: Cannot update an exact search in the databse if it has not already been entered, id was < 0." << endl;
        return;
    }

    ostringstream query;
    query << "UPDATE exact_search SET";

    query << " genomes_generated = " << genomes_generated
        << ", inserted_genomes = " << inserted_genomes

        << ", node_innovation_count = " << node_innovation_count
        << ", edge_innovation_count = " << edge_innovation_count

        << ", inserted_from_disable_edge = " << inserted_from_disable_edge
        << ", inserted_from_enable_edge = " << inserted_from_enable_edge
        << ", inserted_from_split_edge = " << inserted_from_split_edge
        << ", inserted_from_add_edge = " << inserted_from_add_edge
        << ", inserted_from_change_size = " << inserted_from_change_size
        << ", inserted_from_change_size_x = " << inserted_from_change_size_x
        << ", inserted_from_change_size_y = " << inserted_from_change_size_y
        << ", inserted_from_crossover = " << inserted_from_crossover

        << ", generated_from_disable_edge = " << generated_from_disable_edge
        << ", generated_from_enable_edge = " << generated_from_enable_edge
        << ", generated_from_split_edge = " << generated_from_split_edge
        << ", generated_from_add_edge = " << generated_from_add_edge
        << ", generated_from_change_size = " << generated_from_change_size
        << ", generated_from_change_size_x = " << generated_from_change_size_x
        << ", generated_from_change_size_y = " << generated_from_change_size_y
        << ", generated_from_crossover = " << generated_from_crossover

        << ", generator = '" << generator << "'"
        << ", normal_distribution = '" << normal_distribution << "'"
        << ", rng_long = '" << rng_long << "'"
        << ", rng_double = '" << rng_double << "'"
        << " WHERE id = " << id;

    cout << query.str() << endl;
    mysql_exact_query(query.str());

    //genomes are inserted separately

    //need to insert all_nodes and all_edges
    //a genome id of 0 means that they are not assigned to
    //a particular genome
    for (uint32_t i = 0; i < all_nodes.size(); i++) {
        if (all_nodes.at(i)->get_node_id() < 0) {
            all_nodes.at(i)->export_to_database(id, 0);
        }
    }

    for (uint32_t i = 0; i < all_edges.size(); i++) {
        if (all_edges.at(i)->get_edge_id() < 0) {
            all_edges.at(i)->export_to_database(id, 0);
        }
    }

    if ((int32_t)genomes.size() == population_size) {
        if (sort_by_fitness) {
            double worst_fitness = genomes.back()->get_fitness();
            ostringstream delete_query;
            delete_query << "DELETE FROM cnn_genome WHERE exact_id = " << id << " AND best_error > " << worst_fitness;
            cout << delete_query.str() << endl;
            mysql_exact_query(delete_query.str());
        } else {
            double worst_predictions = genomes.back()->get_best_predictions();
            ostringstream delete_query;
            delete_query << "DELETE FROM cnn_genome WHERE exact_id = " << id << " AND best_predictions < " << worst_predictions;
            cout << delete_query.str() << endl;
            mysql_exact_query(delete_query.str());
        }

        ostringstream delete_node_query;
        delete_node_query << "DELETE FROM cnn_node WHERE exact_id = " << id << " AND genome_id > 0 AND NOT EXISTS(SELECT id FROM cnn_genome WHERE cnn_genome.id = cnn_node.genome_id)";
        cout <<  delete_node_query.str() << endl;
        mysql_exact_query(delete_node_query.str());

        ostringstream delete_edge_query;
        delete_edge_query << "DELETE FROM cnn_edge WHERE exact_id = " << id << " AND genome_id > 0 AND NOT EXISTS(SELECT id FROM cnn_genome WHERE cnn_genome.id = cnn_edge.genome_id)";
        cout <<  delete_edge_query.str() << endl;
        mysql_exact_query(delete_edge_query.str());
    }
}

#endif

EXACT::EXACT(const Images &images, string _samples_filename, int _population_size, int _max_epochs, int _max_genomes, string _output_directory, string _search_name, bool _reset_weights) {
    id = -1;

    search_name = _search_name;
    output_directory = _output_directory;
    samples_filename = _samples_filename;

    reset_weights = _reset_weights;
    max_epochs = _max_epochs;
    max_genomes = _max_genomes;

    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
    //unsigned seed = 10;

    generator = minstd_rand0(seed);
    rng_long = uniform_int_distribution<long>(-numeric_limits<long>::max(), numeric_limits<long>::max());
    rng_double = uniform_real_distribution<double>(0, 1.0);

    node_innovation_count = 0;
    edge_innovation_count = 0;

    inserted_genomes = 0;

    population_size = _population_size;

    number_images = images.get_number_images();
    image_channels = images.get_image_channels();
    image_rows = images.get_image_rows();
    image_cols = images.get_image_cols();
    number_classes = images.get_number_classes();

    inserted_from_disable_edge = 0;
    inserted_from_enable_edge = 0;
    inserted_from_split_edge = 0;
    inserted_from_add_edge = 0;
    inserted_from_change_size = 0;
    inserted_from_change_size_x = 0;
    inserted_from_change_size_y = 0;
    inserted_from_crossover = 0;
    inserted_from_reset_weights = 0;

    generated_from_disable_edge = 0;
    generated_from_enable_edge = 0;
    generated_from_split_edge = 0;
    generated_from_add_edge = 0;
    generated_from_change_size = 0;
    generated_from_change_size_x = 0;
    generated_from_change_size_y = 0;
    generated_from_crossover = 0;
    generated_from_reset_weights = 0;

    genomes_generated = 0;

    initial_mu_min = 0.40;
    initial_mu_max = 0.60;
    mu_min = 0.0;
    mu_max = 0.99;

    initial_mu_delta_min = 0.90;
    initial_mu_delta_max = 0.99;
    mu_delta_min = 0.0;
    mu_delta_max = 1.00;

    initial_learning_rate_min = 0.01;
    initial_learning_rate_max = 0.001;
    learning_rate_min = 0.00000001;
    learning_rate_max = 0.1;

    initial_learning_rate_delta_min = 0.99;
    initial_learning_rate_delta_max = 0.90;
    learning_rate_delta_min = 0.00000001;
    learning_rate_delta_max = 1.0;

    initial_weight_decay_min = 0.001;
    initial_weight_decay_max = 0.0001;
    weight_decay_min = 0.00000000;
    weight_decay_max = 0.1;

    initial_weight_decay_delta_min = 0.99;
    initial_weight_decay_delta_max = 0.90;
    weight_decay_delta_min = 0.00000001;
    weight_decay_delta_max = 1.0;

    initial_input_dropout_probability_min = 0.001;
    initial_input_dropout_probability_max = 0.03;
    input_dropout_probability_min = 0.00000001;
    input_dropout_probability_max = 1.0;

    initial_hidden_dropout_probability_min = 0.40;
    initial_hidden_dropout_probability_max = 0.60;
    hidden_dropout_probability_min = 0.20;
    hidden_dropout_probability_max = 0.80;

    initial_velocity_reset_min = 500;
    initial_velocity_reset_max = 10000;
    velocity_reset_min = 0;
    velocity_reset_max = 60000;


    crossover_rate = 0.20;
    more_fit_parent_crossover = 1.00;
    less_fit_parent_crossover = 0.50;

    sort_by_fitness = true;
    reset_weights_chance = 0.20;

    number_mutations = 3;
    edge_disable = 1.0;
    edge_enable = 2.0;
    edge_split = 2.0;
    edge_add = 4.0;
    edge_change_stride = 0.0;
    node_change_size = 1.0;
    node_change_size_x = 0.5;
    node_change_size_y = 0.5;
    node_change_pool_size = 0.0;

    cout << "EXACT settings: " << endl;

    cout << "\tinitial_mu_min: " << initial_mu_min << endl;
    cout << "\tinitial_mu_max: " << initial_mu_max << endl;
    cout << "\tmu_min: " << mu_min << endl;
    cout << "\tmu_max: " << mu_max << endl;

    cout << "\tinitial_mu_delta_min: " << initial_mu_delta_min << endl;
    cout << "\tinitial_mu_delta_max: " << initial_mu_delta_max << endl;
    cout << "\tmu_delta_min: " << mu_delta_min << endl;
    cout << "\tmu_delta_max: " << mu_delta_max << endl;

    cout << "\tinitial_learning_rate_min: " << initial_learning_rate_min << endl;
    cout << "\tinitial_learning_rate_max: " << initial_learning_rate_max << endl;
    cout << "\tlearning_rate_min: " << learning_rate_min << endl;
    cout << "\tlearning_rate_max: " << learning_rate_max << endl;

    cout << "\tinitial_learning_rate_delta_min: " << initial_learning_rate_delta_min << endl;
    cout << "\tinitial_learning_rate_delta_max: " << initial_learning_rate_delta_max << endl;
    cout << "\tlearning_rate_delta_min: " << learning_rate_delta_min << endl;
    cout << "\tlearning_rate_delta_max: " << learning_rate_delta_max << endl;

    cout << "\tinitial_weight_decay_min: " << initial_weight_decay_min << endl;
    cout << "\tinitial_weight_decay_max: " << initial_weight_decay_max << endl;
    cout << "\tweight_decay_min: " << weight_decay_min << endl;
    cout << "\tweight_decay_max: " << weight_decay_max << endl;

    cout << "\tinitial_weight_decay_delta_min: " << initial_weight_decay_delta_min << endl;
    cout << "\tinitial_weight_decay_delta_max: " << initial_weight_decay_delta_max << endl;
    cout << "\tweight_decay_delta_min: " << weight_decay_delta_min << endl;
    cout << "\tweight_decay_delta_max: " << weight_decay_delta_max << endl;

    cout << "\tinitial_input_dropout_probability_min: " << initial_input_dropout_probability_min << endl;
    cout << "\tinitial_input_dropout_probability_max: " << initial_input_dropout_probability_max << endl;
    cout << "\tinput_dropout_probability_min: " << input_dropout_probability_min << endl;
    cout << "\tinput_dropout_probability_max: " << input_dropout_probability_max << endl;

    cout << "\tinitial_hidden_dropout_probability_min: " << initial_hidden_dropout_probability_min << endl;
    cout << "\tinitial_hidden_dropout_probability_max: " << initial_hidden_dropout_probability_max << endl;
    cout << "\thidden_dropout_probability_min: " << hidden_dropout_probability_min << endl;
    cout << "\thidden_dropout_probability_max: " << hidden_dropout_probability_max << endl;

    cout << "\tinitial_velocity_reset_min: " << initial_velocity_reset_min << endl;
    cout << "\tinitial_velocity_reset_max: " << initial_velocity_reset_max << endl;
    cout << "\tvelocity_reset_min: " << velocity_reset_min << endl;
    cout << "\tvelocity_reset_max: " << velocity_reset_max << endl;

    cout << "\tmax_epochs: " << max_epochs << endl;
    cout << "\treset_weights_chance: " << reset_weights_chance << endl;

    cout << "\tcrossover_settings: " << endl;
    cout << "\t\tcrossover_rate: " << crossover_rate << endl;
    cout << "\t\tmore_fit_parent_crossover: " << more_fit_parent_crossover << endl;
    cout << "\t\tless_fit_parent_crossover: " << less_fit_parent_crossover << endl;

    cout << "\tmutation_settings: " << endl;
    cout << "\t\tnumber_mutations: " << number_mutations << endl;
    cout << "\t\tedge_disable: " << edge_disable << endl;
    cout << "\t\tedge_split: " << edge_split << endl;
    cout << "\t\tedge_add: " << edge_add << endl;
    cout << "\t\tedge_change_stride: " << edge_change_stride << endl;
    cout << "\t\tnode_change_size: " << node_change_size << endl;
    cout << "\t\tnode_change_size_x: " << node_change_size_x << endl;
    cout << "\t\tnode_change_size_y: " << node_change_size_y << endl;
    cout << "\t\tnode_change_pool_size: " << node_change_pool_size << endl;

    double total = edge_disable + edge_enable + edge_split + edge_add + edge_change_stride +
                   node_change_size + node_change_size_x + node_change_size_y + node_change_pool_size;

    edge_disable /= total;
    edge_enable /= total;
    edge_split /= total;
    edge_add /= total;
    edge_change_stride /= total;
    node_change_size /= total;
    node_change_size_x /= total;
    node_change_size_y /= total;
    node_change_pool_size /= total;

    cout << "mutation probabilities: " << endl;
    cout << "\tedge_disable: " << edge_disable << endl;
    cout << "\tedge_split: " << edge_split << endl;
    cout << "\tedge_add: " << edge_add << endl;
    cout << "\tedge_change_stride: " << edge_change_stride << endl;
    cout << "\tnode_change_size: " << node_change_size << endl;
    cout << "\tnode_change_size_x: " << node_change_size_x << endl;
    cout << "\tnode_change_size_y: " << node_change_size_y << endl;
    cout << "\tnode_change_pool_size: " << node_change_pool_size << endl;

    if (output_directory.compare("") != 0) {
        write_statistics_header();
        write_hyperparameters_header();
    }
}

int EXACT::get_id() const {
    return id;
}

int EXACT::get_inserted_genomes() const {
    return inserted_genomes;
}

int EXACT::get_max_genomes() const {
    return max_genomes;
}


string EXACT::get_search_name() const {
    return search_name;
}

string EXACT::get_output_directory() const {
    return output_directory;
}

string EXACT::get_samples_filename() const {
    return samples_filename;
}


int EXACT::get_number_images() const {
    return number_images;
}

CNN_Genome* EXACT::get_best_genome() {
    return genomes[0];
}

void EXACT::generate_initial_hyperparameters(double &mu, double &mu_delta, double &learning_rate, double &learning_rate_delta, double &weight_decay, double &weight_decay_delta, double &input_dropout_probability, double &hidden_dropout_probability, int &velocity_reset) {
    mu = (rng_double(generator) * (initial_mu_max - initial_mu_min)) + initial_mu_min;
    mu_delta = (rng_double(generator) * (initial_mu_delta_max - initial_mu_delta_min)) + initial_mu_delta_min;

    learning_rate = (rng_double(generator) * (initial_learning_rate_max - initial_learning_rate_min)) + initial_learning_rate_min;
    learning_rate_delta = (rng_double(generator) * (initial_learning_rate_delta_max - initial_learning_rate_delta_min)) + initial_learning_rate_delta_min;

    weight_decay = (rng_double(generator) * (initial_weight_decay_max - initial_weight_decay_min)) + initial_weight_decay_min;
    weight_decay_delta = (rng_double(generator) * (initial_weight_decay_delta_max - initial_weight_decay_delta_min)) + initial_weight_decay_delta_min;

    input_dropout_probability = (rng_double(generator) * (initial_input_dropout_probability_max - initial_input_dropout_probability_min)) + initial_input_dropout_probability_min;

    hidden_dropout_probability = (rng_double(generator) * (initial_hidden_dropout_probability_max - initial_hidden_dropout_probability_min)) + initial_hidden_dropout_probability_min;

    velocity_reset = (rng_double(generator) * (initial_velocity_reset_max - initial_velocity_reset_min)) + initial_velocity_reset_min;

    cout << "\tGenerated RANDOM hyperparameters:" << endl;
    cout << "\t\tmu: " << mu << endl;
    cout << "\t\tmu_delta: " << mu_delta << endl;
    cout << "\t\tlearning_rate: " << learning_rate << endl;
    cout << "\t\tlearning_rate_delta: " << learning_rate_delta << endl;
    cout << "\t\tweight_decay: " << weight_decay << endl;
    cout << "\t\tweight_decay_delta: " << weight_decay_delta << endl;
    cout << "\t\tinput_dropout_probability: " << input_dropout_probability << endl;
    cout << "\t\thidden_dropout_probability: " << hidden_dropout_probability << endl;
    cout << "\t\tvelocity_reset: " << velocity_reset << endl;
}

void EXACT::generate_simplex_hyperparameters(double &mu, double &mu_delta, double &learning_rate, double &learning_rate_delta, double &weight_decay, double &weight_decay_delta, double &input_dropout_probability, double &hidden_dropout_probability, int &velocity_reset) {

    double best_mu, best_mu_delta, best_learning_rate, best_learning_rate_delta, best_weight_decay, best_weight_decay_delta, best_input_dropout_probability, best_hidden_dropout_probability, best_velocity_reset;

    //get best hyperparameters
    CNN_Genome *best_genome = genomes[0];
    best_mu = best_genome->get_initial_mu();
    best_mu_delta = best_genome->get_mu_delta();
    best_learning_rate = best_genome->get_initial_learning_rate();
    best_learning_rate_delta = best_genome->get_learning_rate_delta();
    best_weight_decay = best_genome->get_initial_weight_decay();
    best_weight_decay_delta = best_genome->get_weight_decay_delta();
    best_input_dropout_probability = best_genome->get_input_dropout_probability();
    best_hidden_dropout_probability = best_genome->get_hidden_dropout_probability();
    best_velocity_reset = best_genome->get_velocity_reset();

    //get average parameters
    double avg_mu, avg_mu_delta, avg_learning_rate, avg_learning_rate_delta, avg_weight_decay, avg_weight_decay_delta, avg_input_dropout_probability, avg_hidden_dropout_probability, avg_velocity_reset;

    avg_mu = 0;
    avg_mu_delta = 0;
    avg_learning_rate = 0;
    avg_learning_rate_delta = 0;
    avg_weight_decay = 0;
    avg_weight_decay_delta = 0;
    avg_input_dropout_probability = 0;
    avg_hidden_dropout_probability = 0;
    avg_learning_rate = 0;
    avg_velocity_reset = 0;

    int simplex_count = 5;
    for (uint32_t i = 0; i < simplex_count; i++) {
        CNN_Genome *current_genome = genomes[rng_double(generator) * genomes.size()];

        avg_mu += current_genome->get_initial_mu();
        avg_mu_delta += current_genome->get_mu_delta();
        avg_learning_rate += current_genome->get_initial_learning_rate();
        avg_learning_rate_delta += current_genome->get_learning_rate_delta();
        avg_weight_decay += current_genome->get_initial_weight_decay();
        avg_weight_decay_delta += current_genome->get_weight_decay_delta();
        avg_input_dropout_probability += current_genome->get_input_dropout_probability();
        avg_hidden_dropout_probability += current_genome->get_hidden_dropout_probability();
        avg_velocity_reset += current_genome->get_velocity_reset();
    }

    avg_mu /= simplex_count;
    avg_mu_delta /= simplex_count;
    avg_learning_rate /= simplex_count;
    avg_learning_rate_delta /= simplex_count;
    avg_weight_decay /= simplex_count;
    avg_weight_decay_delta /= simplex_count;
    avg_input_dropout_probability /= simplex_count;
    avg_hidden_dropout_probability /= simplex_count;
    avg_learning_rate /= simplex_count;
    avg_velocity_reset /= simplex_count;

    double scale = (rng_double(generator) * 2.0) - 0.5;

    mu = avg_mu + ((best_mu - avg_mu) * scale);
    mu_delta = avg_mu_delta + ((best_mu_delta - avg_mu_delta) * scale);
    learning_rate = avg_learning_rate + ((best_learning_rate - avg_learning_rate) * scale);
    learning_rate_delta = avg_learning_rate_delta + ((best_learning_rate_delta - avg_learning_rate_delta) * scale);
    weight_decay = avg_weight_decay + ((best_weight_decay - avg_weight_decay) * scale);
    weight_decay_delta = avg_weight_decay_delta + ((best_weight_decay_delta - avg_weight_decay_delta) * scale);
    input_dropout_probability = avg_input_dropout_probability + ((best_input_dropout_probability - avg_input_dropout_probability) * scale);
    hidden_dropout_probability = avg_hidden_dropout_probability + ((best_hidden_dropout_probability - avg_hidden_dropout_probability) * scale);
    learning_rate = avg_learning_rate + ((best_learning_rate - avg_learning_rate) * scale);
    velocity_reset = avg_velocity_reset + ((best_velocity_reset - avg_velocity_reset) * scale);

    if (mu < mu_min) mu = mu_min;
    if (mu > mu_max) mu = mu_max;
    if (mu_delta < mu_delta_min) mu_delta = mu_delta_min;
    if (mu_delta > mu_delta_max) mu_delta = mu_delta_max;

    if (learning_rate < learning_rate_min) learning_rate = learning_rate_min;
    if (learning_rate > learning_rate_max) learning_rate = learning_rate_max;
    if (learning_rate_delta < learning_rate_delta_min) learning_rate_delta = learning_rate_delta_min;
    if (learning_rate_delta > learning_rate_delta_max) learning_rate_delta = learning_rate_delta_max;

    if (weight_decay < weight_decay_min) weight_decay = weight_decay_min;
    if (weight_decay > weight_decay_max) weight_decay = weight_decay_max;
    if (weight_decay_delta < weight_decay_delta_min) weight_decay_delta = weight_decay_delta_min;
    if (weight_decay_delta > weight_decay_delta_max) weight_decay_delta = weight_decay_delta_max;

    if (input_dropout_probability < input_dropout_probability_min) input_dropout_probability = input_dropout_probability_min;
    if (input_dropout_probability > input_dropout_probability_max) input_dropout_probability = input_dropout_probability_max;

    if (hidden_dropout_probability < hidden_dropout_probability_min) hidden_dropout_probability = hidden_dropout_probability_min;
    if (hidden_dropout_probability > hidden_dropout_probability_max) hidden_dropout_probability = hidden_dropout_probability_max;

    if (velocity_reset < velocity_reset_min) velocity_reset = velocity_reset_min;
    if (velocity_reset > velocity_reset_max) velocity_reset = velocity_reset_max;

    cout << "\tGenerated SIMPLEX hyperparameters:" << endl;
    cout << "\t\tscale: " << scale << endl;
    cout << "\t\tmu: " << mu << endl;
    cout << "\t\tmu_delta: " << mu_delta << endl;
    cout << "\t\tlearning_rate: " << learning_rate << endl;
    cout << "\t\tlearning_rate_delta: " << learning_rate_delta << endl;
    cout << "\t\tweight_decay: " << weight_decay << endl;
    cout << "\t\tweight_decay_delta: " << weight_decay_delta << endl;
    cout << "\t\tinput_dropout_probability: " << input_dropout_probability << endl;
    cout << "\t\thidden_dropout_probability: " << hidden_dropout_probability << endl;
    cout << "\t\tvelocity_reset: " << velocity_reset << endl;
}



CNN_Genome* EXACT::generate_individual() {
    if (inserted_genomes >= max_genomes) return NULL;

    CNN_Genome *genome = NULL;
    if (genomes.size() == 0) {
        //generate the initial minimal CNN
        
        vector<CNN_Node*> input_nodes;
        for (int32_t i = 0; i < image_channels; i++) {
            CNN_Node *input_node = new CNN_Node(node_innovation_count, 0, image_rows, image_cols, INPUT_NODE);
            node_innovation_count++;
            all_nodes.push_back(input_node);
            input_nodes.push_back(input_node);
        }

        for (int32_t i = 0; i < number_classes; i++) {
            CNN_Node *softmax_node = new CNN_Node(node_innovation_count, 1, 1, 1, SOFTMAX_NODE);
            node_innovation_count++;
            all_nodes.push_back(softmax_node);
        }

        for (int32_t i = 0; i < number_classes; i++) {
            for (int32_t j = 0; j < image_channels; j++) {
                CNN_Edge *edge = new CNN_Edge(input_nodes[j], all_nodes[i + image_channels] /*ith softmax node*/, true, edge_innovation_count);

                all_edges.push_back(edge);

                edge_innovation_count++;
            }
        }

        long genome_seed = rng_long(generator);
        //cout << "seeding genome with: " << genome_seed << endl;

        double mu, mu_delta, learning_rate, learning_rate_delta, weight_decay, weight_decay_delta, input_dropout_probability, hidden_dropout_probability;
        int velocity_reset;

        generate_initial_hyperparameters(mu, mu_delta, learning_rate, learning_rate_delta, weight_decay, weight_decay_delta, input_dropout_probability, hidden_dropout_probability, velocity_reset);

        genome = new CNN_Genome(genomes_generated++, genome_seed, max_epochs, reset_weights, velocity_reset, mu, mu_delta, learning_rate, learning_rate_delta, weight_decay, weight_decay_delta, input_dropout_probability, hidden_dropout_probability, all_nodes, all_edges);

    } else if ((int32_t)genomes.size() < population_size) {
        //generate random mutatinos until genomes.size() < population_size
        while (genome == NULL) {
            genome = create_mutation();

            if (!genome->outputs_connected()) {
                cout << "\tAll softmax nodes were not reachable, deleting genome." << endl;
                delete genome;
                genome = NULL;
            }
         }
    } else {
        if (rng_double(generator) < crossover_rate) {
            //generate a child from crossover
            while (genome == NULL) {
                genome = create_child();

                if (!genome->outputs_connected()) {
                    cout << "\tAll softmax nodes were not reachable, deleting genome." << endl;
                    delete genome;
                    genome = NULL;
                }
            }

        } else {
            //generate a mutation
            while (genome == NULL) {
                genome = create_mutation();

                if (!genome->outputs_connected()) {
                    cout << "\tAll softmax nodes were not reachable, deleting genome." << endl;
                    delete genome;
                    genome = NULL;
                }
            }
        }
    }

    genome->initialize();

    if (!genome->sanity_check(SANITY_CHECK_AFTER_GENERATION)) {
        cout << "ERROR: genome " << genome->get_generation_id() << " failed sanity check in generate individual!" << endl;
        exit(1);
    }

    if ((int32_t)genomes.size() < population_size) {
        //insert a copy with a bad fitness so we have more things to generate new genomes with
        CNN_Genome *genome_copy = new CNN_Genome(genomes_generated++, /*new random seed*/ rng_long(generator), max_epochs, reset_weights, genome->get_velocity_reset(), genome->get_initial_mu(), genome->get_mu_delta(), genome->get_initial_learning_rate(), genome->get_learning_rate_delta(), genome->get_initial_weight_decay(), genome->get_weight_decay_delta(), genome->get_input_dropout_probability(), genome->get_hidden_dropout_probability(), genome->get_nodes(), genome->get_edges());

        //for more variability in the initial population, re-initialize weights and bias for these unevaluated copies

        insert_genome(genome_copy);
    }

    return genome;
}

int32_t EXACT::population_contains(CNN_Genome *genome) const {
    for (int32_t i = 0; i < (int32_t)genomes.size(); i++) {
        //we can overwrite genomes that were inserted in the initialization phase
        //and not evaluated
        //if (genomes[i]->get_fitness() == EXACT_MAX_DOUBLE) continue;

        if (genomes[i]->equals(genome)) {
            cout << "\tgenome was the same as genome with generation id: " << genomes[i]->get_generation_id() << endl;
            return i;
        }
    }

    return -1;
}

string parse_fitness(double fitness) {
    if (fitness == EXACT_MAX_DOUBLE) {
        return "UNEVALUATED";
    } else {
        return to_string(fitness);
    }
}

bool EXACT::insert_genome(CNN_Genome* genome) {
    double new_fitness = genome->get_fitness();
    int new_generation_id = genome->get_generation_id();

    bool was_inserted = true;

    inserted_genomes++;

    generated_from_disable_edge += genome->get_generated_by_disable_edge();
    generated_from_enable_edge += genome->get_generated_by_enable_edge();
    generated_from_split_edge += genome->get_generated_by_split_edge();
    generated_from_add_edge += genome->get_generated_by_add_edge();
    generated_from_change_size += genome->get_generated_by_change_size();
    generated_from_change_size_x += genome->get_generated_by_change_size_x();
    generated_from_change_size_y += genome->get_generated_by_change_size_y();
    generated_from_crossover += genome->get_generated_by_crossover();
    generated_from_reset_weights += genome->get_generated_by_reset_weights();

    cout << "genomes evaluated: " << setw(10) << inserted_genomes << ", inserting: " << parse_fitness(genome->get_fitness()) << endl;

    int32_t duplicate_genome = population_contains(genome);
    if (duplicate_genome >= 0) {
        //TODO: if fitness is better, replace this genome with new one
        cout << "found duplicate at position: " << duplicate_genome << endl;

        CNN_Genome *duplicate = genomes[duplicate_genome];
        if (duplicate->get_fitness() > genome->get_fitness()) {
            //erase the genome with loewr fitness from the vector;
            cout << "REPLACING DUPLICATE GENOME, original fitness: " << parse_fitness(duplicate->get_fitness()) << ", new fitness: " << parse_fitness(genome->get_fitness()) << endl;
            genomes.erase(genomes.begin() + duplicate_genome);
            delete duplicate;

        } else {
            cerr << "\tpopulation already contains genome! not inserting." << endl;
            delete genome;

            if (output_directory.compare("") != 0) write_statistics(new_generation_id, new_fitness);
            return false;
        }
    }

    cout << "performing sanity check." << endl;

    if (!genome->sanity_check(SANITY_CHECK_BEFORE_INSERT)) {
        cout << "ERROR: genome " << genome->get_generation_id() << " failed sanity check before insert!" << endl;
        exit(1);
    }
    cout << "genome " << genome->get_generation_id() << " passed sanity check with fitness: " << parse_fitness(genome->get_fitness()) << endl;

    if (genomes.size() == 0 || genome->get_fitness() < genomes[0]->get_fitness()) {
        cout << "new best fitness!" << endl;

        cout << "writing new best (data) to: " << (output_directory + "/global_best_" + to_string(inserted_genomes) + ".txt") << endl;

        genome->write_to_file(output_directory + "/global_best_" + to_string(inserted_genomes) + ".txt");

        cout << "writing new best (graphviz) to: " << (output_directory + "/global_best_" + to_string(inserted_genomes) + ".txt") << endl;

        ofstream gv_file(output_directory + "/global_best_" + to_string(inserted_genomes) + ".gv");
        gv_file << "#EXACT settings: " << endl;

        gv_file << "#EXACT settings: " << endl;

        gv_file << "#\tinitial_mu_min: " << initial_mu_min << endl;
        gv_file << "#\tinitial_mu_max: " << initial_mu_max << endl;
        gv_file << "#\tmu_min: " << mu_min << endl;
        gv_file << "#\tmu_max: " << mu_max << endl;

        gv_file << "#\tinitial_mu_delta_min: " << initial_mu_delta_min << endl;
        gv_file << "#\tinitial_mu_delta_max: " << initial_mu_delta_max << endl;
        gv_file << "#\tmu_delta_min: " << mu_delta_min << endl;
        gv_file << "#\tmu_delta_max: " << mu_delta_max << endl;

        gv_file << "#\tinitial_learning_rate_min: " << initial_learning_rate_min << endl;
        gv_file << "#\tinitial_learning_rate_max: " << initial_learning_rate_max << endl;
        gv_file << "#\tlearning_rate_min: " << learning_rate_min << endl;
        gv_file << "#\tlearning_rate_max: " << learning_rate_max << endl;

        gv_file << "#\tinitial_learning_rate_delta_min: " << initial_learning_rate_delta_min << endl;
        gv_file << "#\tinitial_learning_rate_delta_max: " << initial_learning_rate_delta_max << endl;
        gv_file << "#\tlearning_rate_delta_min: " << learning_rate_delta_min << endl;
        gv_file << "#\tlearning_rate_delta_max: " << learning_rate_delta_max << endl;

        gv_file << "#\tinitial_weight_decay_min: " << initial_weight_decay_min << endl;
        gv_file << "#\tinitial_weight_decay_max: " << initial_weight_decay_max << endl;
        gv_file << "#\tweight_decay_min: " << weight_decay_min << endl;
        gv_file << "#\tweight_decay_max: " << weight_decay_max << endl;

        gv_file << "#\tinitial_weight_decay_delta_min: " << initial_weight_decay_delta_min << endl;
        gv_file << "#\tinitial_weight_decay_delta_max: " << initial_weight_decay_delta_max << endl;
        gv_file << "#\tweight_decay_delta_min: " << weight_decay_delta_min << endl;
        gv_file << "#\tweight_decay_delta_max: " << weight_decay_delta_max << endl;

        gv_file << "#\tinitial_input_dropout_probability_min: " << initial_input_dropout_probability_min << endl;
        gv_file << "#\tinitial_input_dropout_probability_max: " << initial_input_dropout_probability_max << endl;
        gv_file << "#\tinput_dropout_probability_min: " << input_dropout_probability_min << endl;
        gv_file << "#\tinput_dropout_probability_max: " << input_dropout_probability_max << endl;

        gv_file << "#\tinitial_hidden_dropout_probability_min: " << initial_hidden_dropout_probability_min << endl;
        gv_file << "#\tinitial_hidden_dropout_probability_max: " << initial_hidden_dropout_probability_max << endl;
        gv_file << "#\thidden_dropout_probability_min: " << hidden_dropout_probability_min << endl;
        gv_file << "#\thidden_dropout_probability_max: " << hidden_dropout_probability_max << endl;

        gv_file << "#\tinitial_velocity_reset_min: " << initial_velocity_reset_min << endl;
        gv_file << "#\tinitial_velocity_reset_max: " << initial_velocity_reset_max << endl;
        gv_file << "#\tvelocity_reset_min: " << velocity_reset_min << endl;
        gv_file << "#\tvelocity_reset_max: " << velocity_reset_max << endl;

        gv_file << "#\tmax_epochs: " << max_epochs << endl;
        gv_file << "#\treset_weights_chance: " << reset_weights_chance << endl;

        gv_file << "#\tcrossover_settings: " << endl;
        gv_file << "#\t\tcrossover_rate: " << crossover_rate << endl;
        gv_file << "#\t\tmore_fit_parent_crossover: " << more_fit_parent_crossover << endl;
        gv_file << "#\t\tless_fit_parent_crossover: " << less_fit_parent_crossover << endl;

        gv_file << "#\tmutation_settings: " << endl;
        gv_file << "#\t\tnumber_mutations: " << number_mutations << endl;
        gv_file << "#\t\tedge_disable: " << edge_disable << endl;
        gv_file << "#\t\tedge_split: " << edge_split << endl;
        gv_file << "#\t\tedge_add: " << edge_add << endl;
        gv_file << "#\t\tedge_change_stride: " << edge_change_stride << endl;
        gv_file << "#\t\tnode_change_size: " << node_change_size << endl;
        gv_file << "#\t\tnode_change_size_x: " << node_change_size_x << endl;
        gv_file << "#\t\tnode_change_size_y: " << node_change_size_y << endl;
        gv_file << "#\t\tnode_change_pool_size: " << node_change_pool_size << endl;

        genome->print_graphviz(gv_file);
        gv_file.close();
    }
    cout << endl;

    if (genomes.size() == 0) {
        cout << "checking if individual should be inserted or not, genomes.size(): " << genomes.size() << ", population_size: " << population_size << ", genome->get_fitness(): " << genome->get_fitness() << ", genomes is empty!" << endl;
    } else {
        cout << "checking if individual should be inserted or not, genomes.size(): " << genomes.size() << ", population_size: " << population_size << ", genome->get_fitness(): " << genome->get_fitness() << ", genomes.back()->get_fitness: " << genomes.back()->get_fitness() << endl;
    }

    if ((int32_t)genomes.size() >= population_size && genome->get_fitness() >= genomes.back()->get_fitness()) {
        //this will not be inserted into the population
        cout << "not inserting genome due to poor fitness" << endl;
        was_inserted = false;
        delete genome;
    } else {
        cout << "updating search statistics" << endl;

        inserted_from_disable_edge += genome->get_generated_by_disable_edge();
        inserted_from_enable_edge += genome->get_generated_by_enable_edge();
        inserted_from_split_edge += genome->get_generated_by_split_edge();
        inserted_from_add_edge += genome->get_generated_by_add_edge();
        inserted_from_change_size += genome->get_generated_by_change_size();
        inserted_from_change_size_x += genome->get_generated_by_change_size_x();
        inserted_from_change_size_y += genome->get_generated_by_change_size_y();
        inserted_from_crossover += genome->get_generated_by_crossover();
        inserted_from_reset_weights += genome->get_generated_by_reset_weights();

        cout << "inserting new genome" << endl;
        //inorder insert the new individual
        if (sort_by_fitness) {
            genomes.insert( upper_bound(genomes.begin(), genomes.end(), genome, sort_genomes_by_fitness()), genome);
        } else {
            genomes.insert( upper_bound(genomes.begin(), genomes.end(), genome, sort_genomes_by_predictions()), genome);
        }

        cout << "inserted the new genome" << endl;

        //delete the worst individual if we've reached the population size
        if ((int32_t)genomes.size() > population_size) {
            cout << "deleting worst genome" << endl;
            CNN_Genome *worst = genomes.back();
            genomes.pop_back();
            delete worst;
        }
    }

    cout << "genome fitnesses:" << endl;
    for (int32_t i = 0; i < (int32_t)genomes.size(); i++) {
        cout << "\t" << setw(4) << i << " -- genome: " << setw(10) << genomes[i]->get_generation_id() << ", "
            << setw(20) << left << "fitness: " << right << setw(15) << setprecision(5) << fixed << parse_fitness(genomes[i]->get_fitness())
            << " (" << genomes[i]->get_best_predictions() << " correct) on epoch: " << genomes[i]->get_best_error_epoch() 
            //<< ", number enabled edges: " << genomes[i]->get_number_enabled_edges()
            //<< ", number nodes: " << genomes[i]->get_number_nodes() << endl;
            << ", mu: " << setw(10) << fixed << setprecision(5) << genomes[i]->get_initial_mu()
            << ", mu_d: " << setw(10) << fixed << setprecision(5) << genomes[i]->get_mu_delta()
            << ", lr: " << setw(10) << fixed << setprecision(5) << genomes[i]->get_initial_learning_rate()
            << ", lr_d: " << setw(10) << fixed << setprecision(5) << genomes[i]->get_learning_rate_delta()
            << ", wd: " << setw(10) << fixed << setprecision(5) << genomes[i]->get_initial_weight_decay()
            << ", wd_d: " << setw(10) << fixed << setprecision(5) << genomes[i]->get_weight_decay_delta()
            << ", i_d: " << setw(10) << fixed << setprecision(5) << genomes[i]->get_input_dropout_probability()
            << ", h_d: " << setw(10) << fixed << setprecision(5) << genomes[i]->get_hidden_dropout_probability()
            << ", vr: " << setw(10) << fixed << setprecision(5) << genomes[i]->get_velocity_reset()
            << endl;
    }

    /*
    cout << "genome best error: " << endl;
    for (int32_t i = 0; i < (int32_t)genomes.size(); i++) {
        cout << "\t" << setw(4) << i << " -- genome: " << setw(10) << genomes[i]->get_generation_id() << ", ";
        genomes[i]->print_best_error(cout);
    }

    cout << "genome correct predictions: " << endl;
    for (int32_t i = 0; i < (int32_t)genomes.size(); i++) {
        cout << "\t" << setw(4) << i << " -- genome: " << setw(10) << genomes[i]->get_generation_id() << ", ";
        genomes[i]->print_best_predictions(cout);
    }
    */

    cout << endl;

    if (output_directory.compare("") != 0) write_statistics(new_generation_id, new_fitness);
    return was_inserted;
}

CNN_Genome* EXACT::create_mutation() {
    //mutation options:
    //edges:
    //  1. disable edge (but make sure output node is still reachable)
    //  2. split edge
    //  3. add edge (make sure it does not exist already)
    //  4. increase/decrease stride (not yet)
    //nodes:
    //  1. increase/decrease size_x
    //  2. increase/decrease size_y
    //  3. increase/decrease max_pool (not yet)

    long child_seed = rng_long(generator);

    CNN_Genome *parent = genomes[rng_double(generator) * genomes.size()];

    cout << "\tgenerating child " << genomes_generated << " from parent genome: " << parent->get_generation_id() << endl;

    double mu, mu_delta, learning_rate, learning_rate_delta, weight_decay, weight_decay_delta, input_dropout_probability, hidden_dropout_probability;
    int velocity_reset;

    if (inserted_genomes < (population_size * 2)) {
        cout << "\tGenerating hyperparameters randomly." << endl;
        generate_initial_hyperparameters(mu, mu_delta, learning_rate, learning_rate_delta, weight_decay, weight_decay_delta, input_dropout_probability, hidden_dropout_probability, velocity_reset);
    } else {
        cout << "\tGenerating hyperparameters with simplex." << endl;
        generate_simplex_hyperparameters(mu, mu_delta, learning_rate, learning_rate_delta, weight_decay, weight_decay_delta, input_dropout_probability, hidden_dropout_probability, velocity_reset);
    }

    CNN_Genome *child = new CNN_Genome(genomes_generated++, child_seed, max_epochs, reset_weights, velocity_reset, mu, mu_delta, learning_rate, learning_rate_delta, weight_decay, weight_decay_delta, input_dropout_probability, hidden_dropout_probability, parent->get_nodes(), parent->get_edges());

    cout << "\tchild nodes:" << endl;
    for (int32_t i = 0; i < child->get_number_nodes(); i++) {
        cout << "\t\tnode innovation number: " << child->get_node(i)->get_innovation_number() << endl;
    }

    cout << "\tchild edges:" << endl;
    for (int32_t i = 0; i < child->get_number_edges(); i++) {
        cout << "\t\tedge innovation number: " << child->get_edge(i)->get_innovation_number()
            << ", input node innovation number: " << child->get_edge(i)->get_input_innovation_number()
            << ", output node innovation number: " << child->get_edge(i)->get_output_innovation_number()
            << endl;
    }

    if (parent->get_fitness() == EXACT_MAX_DOUBLE) {
        //This parent has not actually been evaluated (the population is still initializing)
        //we can set the best_bias and best_weights randomly so that they are used when it
        //starts up

        cout << "\tparent had not been evaluated yet, but best_bias and best_weights should have been set randomly" << endl;
    } else {
        cout << "\tparent had been evaluated! not setting best_bias and best_weights randomly" << endl;
        cout << "\tparent fitness: " << parent->get_fitness() << endl;
    }

    int modifications = 0;

    while (modifications < number_mutations) {
        double r = rng_double(generator);
        cout << "\tr: " << r << endl;
        r -= 0.00001;

        if (r < edge_disable) {
            cout << "\tDISABLING EDGE!" << endl;

            int edge_position = rng_double(generator) * child->get_number_edges();
            if (child->disable_edge(edge_position)) {
                child->set_generated_by_disable_edge();
                modifications++;
            }

            continue;
        } 
        r -= edge_disable;

        if (r < edge_enable) {
            cout << "\tENABLING EDGE!" << endl;

            vector< CNN_Edge* > disabled_edges;

            for (int32_t i = 0; i < child->get_number_edges(); i++) {
                CNN_Edge* current = child->get_edge(i);

                if (current == NULL) {
                    cout << "ERROR! edge " << i << " became null on child!" << endl;
                    exit(1);
                }

                if (current->is_disabled()) {
                    disabled_edges.push_back(current);
                }
            }
            
            if (disabled_edges.size() > 0) {
                int edge_position = rng_double(generator) * disabled_edges.size();
                CNN_Edge* disabled_edge = disabled_edges[edge_position];

                cout << "\t\tenabling edge: " << disabled_edge->get_innovation_number() << " between input node innovation number " << disabled_edge->get_input_node()->get_innovation_number() << " and output node innovation number " << disabled_edge->get_output_node()->get_innovation_number() << endl;

                disabled_edge->enable();
                //reinitialize weights for re-enabled edge
                disabled_edge->set_needs_init();
                child->set_generated_by_enable_edge();
                modifications++;
            } else {
                cout << "\t\tcould not enable an edge as there were no disabled edges!" << endl;
            }

            continue;
        } 
        r -= edge_enable;


        if (r < edge_split) {
            int edge_position = rng_double(generator) * child->get_number_edges();
            cout << "\tSPLITTING EDGE IN POSITION: " << edge_position << "!" << endl;

            CNN_Edge* edge = child->get_edge(edge_position);

            CNN_Node* input_node = edge->get_input_node();
            CNN_Node* output_node = edge->get_output_node();

            double depth = (input_node->get_depth() + output_node->get_depth()) / 2.0;
            int size_x = (input_node->get_size_x() + output_node->get_size_x()) / 2.0;
            int size_y = (input_node->get_size_y() + output_node->get_size_y()) / 2.0;

            CNN_Node *child_node = new CNN_Node(node_innovation_count, depth, size_x, size_y, HIDDEN_NODE);
            node_innovation_count++;

            //add two new edges, disable the split edge
            cout << "\t\tcreating edge " << edge_innovation_count << endl;
            CNN_Edge *edge1 = new CNN_Edge(input_node, child_node, false, edge_innovation_count);
            edge_innovation_count++;

            cout << "\t\tcreating edge " << edge_innovation_count << endl;
            CNN_Edge *edge2 = new CNN_Edge(child_node, output_node, false, edge_innovation_count);
            edge_innovation_count++;

            cout << "\t\tdisabling edge " << edge->get_innovation_number() << endl;
            edge->disable();

            child->add_node(child_node);
            child->add_edge(edge1);
            child->add_edge(edge2);

            //make sure copies are added to all_edges and all_nodes
            CNN_Node *node_copy = child_node->copy();
            CNN_Edge *edge_copy_1 = edge1->copy();
            CNN_Edge *edge_copy_2 = edge2->copy();

            //insert the new node into the population in sorted order
            all_nodes.insert( upper_bound(all_nodes.begin(), all_nodes.end(), node_copy, sort_CNN_Nodes_by_depth()), node_copy);
            edge_copy_1->set_nodes(all_nodes);
            edge_copy_2->set_nodes(all_nodes);

            all_edges.insert( upper_bound(all_edges.begin(), all_edges.end(), edge_copy_1, sort_CNN_Edges_by_depth()), edge_copy_1);
            all_edges.insert( upper_bound(all_edges.begin(), all_edges.end(), edge_copy_2, sort_CNN_Edges_by_depth()), edge_copy_2);

            child->set_generated_by_split_edge();
            modifications++;

            continue;
        }
        r -= edge_split;

        if (r < edge_add) {
            cout << "\tADDING EDGE!" << endl;

            CNN_Node *node1;
            CNN_Node *node2;

            do {
                int r1 = rng_double(generator) * child->get_number_nodes();
                int r2 = rng_double(generator) * child->get_number_nodes() - 1;

                if (r1 == r2) r2++;

                if (r1 > r2) {  //swap r1 and r2 so node2 is always deeper than node1
                    int temp = r1;
                    r1 = r2;
                    r2 = temp;
                }

                //cout << "child->get_number_nodes(): " <<  child->get_number_nodes() << ", r1: " << r1 << ", r2: " << r2 << endl;

                node1 = child->get_node(r1);
                node2 = child->get_node(r2);
            } while (node1->get_depth() >= node2->get_depth());
            //after this while loop, node 2 will always be deeper than node 1

            int node1_innovation_number = node1->get_innovation_number();
            int node2_innovation_number = node2->get_innovation_number();

            //check to see if the edge already exists
            bool edge_exists = false;
            int all_edges_position = -1;
            for (int32_t i = 0; i < (int32_t)all_edges.size(); i++) {
                if (all_edges.at(i)->connects(node1_innovation_number, node2_innovation_number)) {
                    edge_exists = true;
                    all_edges_position = i;
                    break;
                }
            }

            bool edge_exists_in_child = false;
            for (int32_t i = 0; i < child->get_number_edges(); i++) {
                if (child->get_edge(i)->connects(node1_innovation_number, node2_innovation_number)) {
                    edge_exists_in_child = true;
                    break;
                }
            }

            if (edge_exists && !edge_exists_in_child) {
                //edge exists in another genome, copy from all_edges
                //we know the child has both endpoints because we grabbed node1 and node2 from the child
                cout << "\t\tcopying edge in position " << all_edges_position << " from all_edges!" << endl;
                CNN_Edge *edge_copy = all_edges.at(all_edges_position)->copy();
                cout << "\t\tedge_copy->input_innovation_number: " << edge_copy->get_input_innovation_number() << endl;
                cout << "\t\tedge_copy->output_innovation_number: " << edge_copy->get_output_innovation_number() << endl;

                //enable the edge in case it was disabled
                edge_copy->enable();
                if (!edge_copy->set_nodes(child->get_nodes())) {
                    edge_copy->resize();
                }

                child->add_edge( edge_copy );

                child->set_generated_by_add_edge();
                modifications++;
            } else if (!edge_exists && !edge_exists_in_child) {
                //edge does not exist at all
                cout << "\t\tadding edge between node innovation numbers " << node1_innovation_number << " and " << node2_innovation_number << endl;

                CNN_Edge *edge = new CNN_Edge(node1, node2, false, edge_innovation_count);
                edge_innovation_count++;
                //insert edge in order of depth

                //enable the edge in case it was disabled
                edge->enable();
                child->add_edge(edge);

                CNN_Edge *edge_copy = edge->copy();
                edge_copy->set_nodes(all_nodes);

                all_edges.insert( upper_bound(all_edges.begin(), all_edges.end(), edge_copy, sort_CNN_Edges_by_depth()), edge_copy);

                child->set_generated_by_add_edge();

                modifications++;
            } else {
                cout << "\t\tnot adding edge between node innovation numbers " << node1_innovation_number << " and " << node2_innovation_number << " because edge already exists!" << endl;
            }

            continue;
        }
        r -= edge_add;

        if (r < edge_change_stride) {
            cout << "\tCHANGING EDGE STRIDE!" << endl;

            //child->mutate(MUTATE_EDGE_STRIDE, node_innovation_count, edge_innovation_count);

            continue;
        }
        r -= edge_change_stride;

        if (r < node_change_size) {
            cout << "\tCHANGING NODE SIZE X and Y!" << endl;

            if (child->get_number_softmax_nodes() + child->get_number_input_nodes() == child->get_number_nodes()) {
                cout << "\t\tno non-input or softmax nodes so cannot change node size" << endl;
                continue;
            }

            //should have a value between -2 and 2 (inclusive)
            int change = (2 * rng_double(generator)) + 1;
            if (rng_double(generator) < 0.5) change *= -1;

            //make sure we don't change the size of the input node
            cout << "\t\tnumber nodes: " << child->get_number_nodes() << endl;
            cout << "\t\tnumber input nodes: " << child->get_number_input_nodes() << endl;
            cout << "\t\tnumber softmax nodes: " << child->get_number_softmax_nodes() << endl;
            int r = (rng_double(generator) * (child->get_number_nodes() - child->get_number_input_nodes() - child->get_number_softmax_nodes())) + child->get_number_input_nodes();
            cout << "\t\tr: " << r << endl;

            CNN_Node *modified_node = child->get_node(r);
            cout << "\t\tselected node: " << r << " with innovation number: " << modified_node->get_innovation_number() << endl;

            if (modified_node->is_input()) {
                cout << "\t\tmodified node was input, this should never happen!" << endl;
                exit(1);
            }

            if (modified_node->is_softmax()) {
                cout << "\t\tmodified node was softmax, this should never happen!" << endl;
                exit(1);
            }

            int previous_size_x = modified_node->get_size_x();
            int previous_size_y = modified_node->get_size_y();
            cout << "\t\tsize x before resize: " << previous_size_x << " modifying by change: " << change << endl;
            cout << "\t\tsize y before resize: " << previous_size_y << " modifying by change: " << change << endl;

            bool modified_x = modified_node->modify_size_x(change);
            bool modified_y = modified_node->modify_size_y(change);

            if (modified_x || modified_y) {
                //need to make sure all edges with this as it's input or output get updated
                cout << "\t\tresizing edges around node: " << modified_node->get_innovation_number() << endl;

                child->resize_edges_around_node( modified_node->get_innovation_number() );
                child->set_generated_by_change_size();
                modifications++;

                cout << "\t\tmodified size x by " << change << " from " << previous_size_x << " to " << modified_node->get_size_x() << endl;
                cout << "\t\tmodified size y by " << change << " from " << previous_size_y << " to " << modified_node->get_size_y() << endl;
            } else {
                cout << "\t\tmodification resulted in no change" << endl;
            }

            continue;
        }
        r -= node_change_size;

        if (r < node_change_size_x) {
            cout << "\tCHANGING NODE SIZE X!" << endl;

            if (child->get_number_softmax_nodes() + child->get_number_input_nodes() == child->get_number_nodes()) {
                cout << "\t\tno non-input or softmax nodes so cannot change node size" << endl;
                continue;
            }

            //should have a value between -2 and 2 (inclusive)
            int change = (2 * rng_double(generator)) + 1;
            if (rng_double(generator) < 0.5) change *= -1;

            //make sure we don't change the size of the input node
            cout << "\t\tnumber nodes: " << child->get_number_nodes() << endl;
            cout << "\t\tnumber input nodes: " << child->get_number_input_nodes() << endl;
            cout << "\t\tnumber softmax nodes: " << child->get_number_softmax_nodes() << endl;
            int r = (rng_double(generator) * (child->get_number_nodes() - child->get_number_input_nodes() - child->get_number_softmax_nodes())) + child->get_number_input_nodes();
 
            CNN_Node *modified_node = child->get_node(r);
            cout << "\t\tselected node: " << r << " with innovation number: " << modified_node->get_innovation_number() << endl;

            if (modified_node->is_input()) {
                cout << "\t\tmodified node was input, this should never happen!" << endl;
                exit(1);
            }

            if (modified_node->is_softmax()) {
                cout << "\t\tmodified node was softmax, this should never happen!" << endl;
                exit(1);
            }


            int previous_size_x = modified_node->get_size_x();
            cout << "\t\tsize x before resize: " << previous_size_x << " modifying by change: " << change << endl;

            if (modified_node->modify_size_x(change)) {
                //need to make sure all edges with this as it's input or output get updated
                cout << "\t\tresizing edges around node: " << modified_node->get_innovation_number() << endl;

                child->resize_edges_around_node( modified_node->get_innovation_number() );
                child->set_generated_by_change_size_x();
                modifications++;

                cout << "\t\tmodified size x by " << change << " from " << previous_size_x << " to " << modified_node->get_size_x() << endl;
            } else {
                cout << "\t\tmodification resulted in no change" << endl;
            }

            continue;
        }
        r -= node_change_size_x;

        if (r < node_change_size_y) {
            cout << "\tCHANGING NODE SIZE Y!" << endl;

            if (child->get_number_softmax_nodes() + child->get_number_input_nodes() == child->get_number_nodes()) {
                cout << "\t\tno non-input or softmax nodes so cannot change node size" << endl;
                continue;
            }

            //should have a value between -2 and 2 (inclusive)
            int change = (2 * rng_double(generator)) + 1;
            if (rng_double(generator) < 0.5) change *= -1;

            //make sure we don't change the size of the input node
            cout << "\t\tnumber nodes: " << child->get_number_nodes() << endl;
            cout << "\t\tnumber input nodes: " << child->get_number_input_nodes() << endl;
            cout << "\t\tnumber softmax nodes: " << child->get_number_softmax_nodes() << endl;
            int r = (rng_double(generator) * (child->get_number_nodes() - child->get_number_input_nodes() - child->get_number_softmax_nodes())) + child->get_number_input_nodes();
 
            CNN_Node *modified_node = child->get_node(r);
            cout << "\t\tselected node: " << r << " with innovation number: " << modified_node->get_innovation_number() << endl;

            if (modified_node->is_input()) {
                cout << "\t\tmodified node was input, this should never happen!" << endl;
                exit(1);
            }

            if (modified_node->is_softmax()) {
                cout << "\t\tmodified node was softmax, this should never happen!" << endl;
                exit(1);
            }


            int previous_size_y = modified_node->get_size_y();
            cout << "\t\tsize y before resize: " << previous_size_y << " modifying by change: " << change << endl;

            if (modified_node->modify_size_y(change)) {
                //need to make sure all edges with this as it's input or output get updated
                cout << "\t\tresizing edges around node: " << modified_node->get_innovation_number() << endl;

                child->resize_edges_around_node( modified_node->get_innovation_number() );
                child->set_generated_by_change_size_y();
                modifications++;

                cout << "\t\tmodified size y by " << change << " from " << previous_size_y << " to " << modified_node->get_size_y() << endl;
            } else {
                cout << "\t\tmodification resulted in no change" << endl;
            }

            continue;
        }
        r -= node_change_size_y;

        if (r < node_change_pool_size) {
            cout << "\tCHANGING NODE POOL SIZE!" << endl;

            //child->mutate(MUTATE_NODE_POOL_SIZE, node_innovation_count, edge_innovation_count);

            continue;
        }
        r -= node_change_pool_size;

        cout << "ERROR: problem choosing mutation type -- should never get here!" << endl;
        cout << "\tremaining random value (for mutation selection): " << r << endl;
        exit(1);
    }

    return child;
}

void attempt_node_insert(vector<CNN_Node*> &nodes, CNN_Node *node) {
    for (int32_t i = 0; i < (int32_t)nodes.size(); i++) {
        if (nodes[i]->get_innovation_number() == node->get_innovation_number()) return;
    }

    nodes.insert( upper_bound(nodes.begin(), nodes.end(), node->copy(), sort_CNN_Nodes_by_depth()), node->copy());
}

bool edges_contains(vector< CNN_Edge* > &edges, CNN_Edge *edge) {
    for (int32_t i = 0; i < (int32_t)edges.size(); i++) {
        if (edges[i]->get_innovation_number() == edge->get_innovation_number()) return true;
    }
    return false;
}

CNN_Genome* EXACT::create_child() {
    cout << "\tCREATING CHILD THROUGH CROSSOVER!" << endl;
    int r1 = rng_double(generator) * genomes.size();
    int r2 = rng_double(generator) * (genomes.size() - 1);
    if (r1 == r2) r2++;

    //parent should have higher fitness
    if (r2 < r1) {
        int tmp = r2;
        r2 = r1;
        r1 = tmp;
    }
    cout << "\t\tparent positions: " << r1 << " and " << r2 << endl;

    CNN_Genome *parent1 = genomes[r1];
    CNN_Genome *parent2 = genomes[r2];

    cout << "\t\tgenerating child " << genomes_generated << " from parents: " << parent1->get_generation_id() << " and " << parent2->get_generation_id() << endl;

    vector< CNN_Node* > child_nodes;
    vector< CNN_Edge* > child_edges;

    int p1_position = 0;
    int p2_position = 0;

    //edges are not sorted in order of innovation number, they need to be
    vector< CNN_Edge* > p1_edges;
    for (int i = 0; i < parent1->get_number_edges(); i++) {
        p1_edges.push_back(parent1->get_edge(i));
    }

    vector< CNN_Edge* > p2_edges;
    for (int i = 0; i < parent2->get_number_edges(); i++) {
        p2_edges.push_back(parent2->get_edge(i));
    }

    sort(p1_edges.begin(), p1_edges.end(), sort_CNN_Edges_by_innovation());
    sort(p2_edges.begin(), p2_edges.end(), sort_CNN_Edges_by_innovation());

    cerr << "p1 innovation numbers AFTER SORT: " << endl;
    for (int32_t i = 0; i < (int32_t)p1_edges.size(); i++) {
        cerr << "\t" << p1_edges[i]->get_innovation_number() << endl;
    }
    cerr << "p2 innovation numbers AFTER SORT: " << endl;
    for (int32_t i = 0; i < (int32_t)p2_edges.size(); i++) {
        cerr << "\t" << p2_edges[i]->get_innovation_number() << endl;
    }


    while (p1_position < (int32_t)p1_edges.size() && p2_position < (int32_t)p2_edges.size()) {
        CNN_Edge* p1_edge = p1_edges[p1_position];
        CNN_Edge* p2_edge = p2_edges[p2_position];

        int p1_innovation = p1_edge->get_innovation_number();
        int p2_innovation = p2_edge->get_innovation_number();

        if (p1_innovation == p2_innovation) {
            CNN_Edge *edge = p1_edge->copy();

            if (edges_contains(child_edges, edge)) {
                cerr << "ERROR in crossover! trying to push an edge with innovation_number: " << edge->get_innovation_number() << " and it already exists in the vector!" << endl;
                cerr << "p1_position: " << p1_position << ", p1_size: " << p1_edges.size() << endl;
                cerr << "p2_position: " << p2_position << ", p2_size: " << p2_edges.size() << endl;
                cerr << "vector innovation numbers: " << endl;
                for (int32_t i = 0; i < (int32_t)child_edges.size(); i++) {
                    cerr << "\t" << child_edges[i]->get_innovation_number() << endl;
                }
            }

            child_edges.push_back(edge->copy());

            //push back surrounding nodes
            attempt_node_insert(child_nodes, p1_edge->get_input_node());
            attempt_node_insert(child_nodes, p1_edge->get_output_node());

            p1_position++;
            p2_position++;
        } else if (p1_innovation < p2_innovation) {
            CNN_Edge *edge = p1_edge->copy();

            if (edges_contains(child_edges, edge)) {
                cerr << "ERROR in crossover! trying to push an edge with innovation_number: " << edge->get_innovation_number() << " and it already exists in the vector!" << endl;
                cerr << "p1_position: " << p1_position << ", p1_size: " << p1_edges.size() << endl;
                cerr << "p2_position: " << p2_position << ", p2_size: " << p2_edges.size() << endl;
                cerr << "vector innovation numbers: " << endl;
                for (int32_t i = 0; i < (int32_t)child_edges.size(); i++) {
                    cerr << "\t" << child_edges[i]->get_innovation_number() << endl;
                }
            }

            child_edges.push_back(edge);

            if (rng_double(generator) >= more_fit_parent_crossover) {
                edge->disable();
            }

            //push back surrounding nodes
            attempt_node_insert(child_nodes, p1_edge->get_input_node());
            attempt_node_insert(child_nodes, p1_edge->get_output_node());

            p1_position++;
        } else {
            CNN_Edge *edge = p2_edge->copy();

            if (edges_contains(child_edges, edge)) {
                cerr << "ERROR in crossover! trying to push an edge with innovation_number: " << edge->get_innovation_number() << " and it already exists in the vector!" << endl;
                cerr << "p1_position: " << p1_position << ", p1_size: " << p1_edges.size() << endl;
                cerr << "p2_position: " << p2_position << ", p2_size: " << p2_edges.size() << endl;
                cerr << "vector innovation numbers: " << endl;
                for (int32_t i = 0; i < (int32_t)child_edges.size(); i++) {
                    cerr << "\t" << child_edges[i]->get_innovation_number() << endl;
                }
            }

            child_edges.push_back(edge);

            if (rng_double(generator) >= less_fit_parent_crossover) {
                edge->disable();
            }

            //push back surrounding nodes
            attempt_node_insert(child_nodes, p2_edge->get_input_node());
            attempt_node_insert(child_nodes, p2_edge->get_output_node());

            p2_position++;
        }
    }

    while (p1_position < (int32_t)p1_edges.size()) {
        CNN_Edge* p1_edge = p1_edges[p1_position];

        CNN_Edge *edge = p1_edge->copy();

        if (edges_contains(child_edges, edge)) {
            cerr << "ERROR in crossover! trying to push an edge with innovation_number: " << edge->get_innovation_number() << " and it already exists in the vector!" << endl;
            cerr << "p1_position: " << p1_position << ", p1_size: " << p1_edges.size() << endl;
            cerr << "p1 innovation numbers: " << endl;
            for (int32_t i = 0; i < (int32_t)p1_edges.size(); i++) {
                cerr << "\t" << p1_edges[i]->get_innovation_number() << endl;
            }
            cerr << "p2_position: " << p2_position << ", p2_size: " << p2_edges.size() << endl;
            cerr << "p2 innovation numbers: " << endl;
            for (int32_t i = 0; i < (int32_t)p2_edges.size(); i++) {
                cerr << "\t" << p2_edges[i]->get_innovation_number() << endl;
            }
            cerr << "vector innovation numbers: " << endl;
            for (int32_t i = 0; i < (int32_t)child_edges.size(); i++) {
                cerr << "\t" << child_edges[i]->get_innovation_number() << endl;
            }
        }

        child_edges.push_back(edge);

        if (rng_double(generator) >= more_fit_parent_crossover) {
            edge->disable();
        }

        //push back surrounding nodes
        attempt_node_insert(child_nodes, p1_edge->get_input_node());
        attempt_node_insert(child_nodes, p1_edge->get_output_node());

        p1_position++;
    }

    while (p2_position < (int32_t)p2_edges.size()) {
        CNN_Edge* p2_edge = p2_edges[p2_position];

        CNN_Edge *edge = p2_edge->copy();

        if (edges_contains(child_edges, edge)) {
            cerr << "ERROR in crossover! trying to push an edge with innovation_number: " << edge->get_innovation_number() << " and it already exists in the vector!" << endl;
            cerr << "p1_position: " << p1_position << ", p1_size: " << p1_edges.size() << endl;
            cerr << "p1 innovation numbers: " << endl;
            for (int32_t i = 0; i < (int32_t)p1_edges.size(); i++) {
                cerr << "\t" << p1_edges[i]->get_innovation_number() << endl;
            }
            cerr << "p2_position: " << p2_position << ", p2_size: " << p2_edges.size() << endl;
            cerr << "p2 innovation numbers: " << endl;
            for (int32_t i = 0; i < (int32_t)p2_edges.size(); i++) {
                cerr << "\t" << p2_edges[i]->get_innovation_number() << endl;
            }
            cerr << "vector innovation numbers: " << endl;
            for (int32_t i = 0; i < (int32_t)child_edges.size(); i++) {
                cerr << "\t" << child_edges[i]->get_innovation_number() << endl;
            }
        }

        child_edges.push_back(edge);

        if (rng_double(generator) >= less_fit_parent_crossover) {
            edge->disable();
        }

        //push back surrounding nodes
        attempt_node_insert(child_nodes, p2_edge->get_input_node());
        attempt_node_insert(child_nodes, p2_edge->get_output_node());

        p2_position++;
    }

    sort(child_edges.begin(), child_edges.end(), sort_CNN_Edges_by_depth());
    sort(child_nodes.begin(), child_nodes.end(), sort_CNN_Nodes_by_depth());

    for (int32_t i = 0; i < (int32_t)child_edges.size(); i++) {
        if (!child_edges[i]->set_nodes(child_nodes)) {
            cout << "\t\treinitializing weights of copy" << endl;
            child_edges[i]->resize();
        }
    }

    long genome_seed = rng_long(generator);

    double mu, mu_delta, learning_rate, learning_rate_delta, weight_decay, weight_decay_delta, input_dropout_probability, hidden_dropout_probability;
    int velocity_reset;

    if (inserted_genomes < (population_size * 2)) {
        cout << "\tGenerating hyperparameters randomly." << endl;
        generate_initial_hyperparameters(mu, mu_delta, learning_rate, learning_rate_delta, weight_decay, weight_decay_delta, input_dropout_probability, hidden_dropout_probability, velocity_reset);
    } else {
        cout << "\tGenerating hyperparameters with simplex." << endl;
        generate_simplex_hyperparameters(mu, mu_delta, learning_rate, learning_rate_delta, weight_decay, weight_decay_delta, input_dropout_probability, hidden_dropout_probability, velocity_reset);
    }

    CNN_Genome *child = new CNN_Genome(genomes_generated++, genome_seed, max_epochs, reset_weights, velocity_reset, mu, mu_delta, learning_rate, learning_rate_delta, weight_decay, weight_decay_delta, input_dropout_probability, hidden_dropout_probability, child_nodes, child_edges);

    child->set_generated_by_crossover();

    return child;
}


void EXACT::write_statistics(int new_generation_id, double new_fitness) {
    double min_fitness = EXACT_MAX_DOUBLE;
    double max_fitness = -EXACT_MAX_DOUBLE;
    double avg_fitness = 0.0;
    int fitness_count = 0;

    double min_epochs = EXACT_MAX_DOUBLE;
    double max_epochs = -EXACT_MAX_DOUBLE;
    double avg_epochs = 0.0;
    int epochs_count = 0;

    for (int32_t i = 0; i < (int32_t)genomes.size(); i++) {
        double fitness = genomes[i]->get_fitness();

        if (fitness != EXACT_MAX_DOUBLE) {
            avg_fitness += fitness;
            fitness_count++;
        }

        if (fitness < min_fitness) min_fitness = fitness;
        if (fitness > max_fitness) max_fitness = fitness;

        double epochs = genomes[i]->get_best_error_epoch();

        if (epochs != EXACT_MAX_DOUBLE) {
            avg_epochs += epochs;
            epochs_count++;
        }

        if (epochs < min_epochs) min_epochs = epochs;
        if (epochs > max_epochs) max_epochs = epochs;
    }
    avg_fitness /= fitness_count;
    avg_epochs /= epochs_count;

    if (fitness_count == 0) avg_fitness = 0.0;
    if (min_fitness == EXACT_MAX_DOUBLE) min_fitness = 0;
    if (max_fitness == EXACT_MAX_DOUBLE) max_fitness = 0;
    if (max_fitness == -EXACT_MAX_DOUBLE) max_fitness = 0;

    if (epochs_count == 0) avg_epochs = 0.0;
    if (min_epochs == EXACT_MAX_DOUBLE) min_epochs = 0;
    if (max_epochs == EXACT_MAX_DOUBLE) max_epochs = 0;
    if (max_epochs == -EXACT_MAX_DOUBLE) max_epochs = 0;

    fstream out(output_directory + "/progress.txt", fstream::out | fstream::app);

    out << setw(16) << new_generation_id
        << setw(16) << new_fitness
        << setw(16) << inserted_genomes
        << setw(16) << setprecision(5) << fixed << min_fitness
        << setw(16) << setprecision(5) << fixed << avg_fitness
        << setw(16) << setprecision(5) << fixed << max_fitness
        << setw(16) << setprecision(5) << fixed << min_epochs
        << setw(16) << setprecision(5) << fixed << avg_epochs
        << setw(16) << setprecision(5) << fixed << max_epochs;

    if (generated_from_disable_edge == 0) {
        out << setw(16) << setprecision(3) << 0.0;
    } else {
        out << setw(16) << setprecision(3) << (100.0 * (double)inserted_from_disable_edge / (double)generated_from_disable_edge);
    }

    if (generated_from_enable_edge == 0) {
        out << setw(16) << setprecision(3) << 0.0;
    } else {
        out << setw(16) << setprecision(3) << (100.0 * (double)inserted_from_enable_edge / (double)generated_from_enable_edge);
    }

    if (generated_from_split_edge == 0) {
        out << setw(16) << setprecision(3) << 0.0;
    } else {
        out << setw(16) << setprecision(3) << (100.0 * (double)inserted_from_split_edge / (double)generated_from_split_edge);
    }

    if (generated_from_add_edge == 0) {
        out << setw(16) << setprecision(3) << 0.0;
    } else {
        out << setw(16) << setprecision(3) << (100.0 * (double)inserted_from_add_edge / (double)generated_from_add_edge);
    }

    if (generated_from_change_size == 0) {
        out << setw(16) << setprecision(3) << 0.0;
    } else {
        out << setw(16) << setprecision(3) << (100.0 * (double)inserted_from_change_size / (double)generated_from_change_size);
    }

    if (generated_from_change_size_x == 0) {
        out << setw(16) << setprecision(3) << 0.0;
    } else {
        out << setw(16) << setprecision(3) << (100.0 * (double)inserted_from_change_size_x / (double)generated_from_change_size_x);
    }

    if (generated_from_change_size_y == 0) {
        out << setw(16) << setprecision(3) << 0.0;
    } else {
        out << setw(16) << setprecision(3) << (100.0 * (double)inserted_from_change_size_y / (double)generated_from_change_size_y);
    }

    if (generated_from_crossover == 0) {
        out << setw(16) << setprecision(3) << 0.0;
    } else {
        out << setw(16) << setprecision(3) << (100.0 * (double)inserted_from_crossover / (double)generated_from_crossover);
    }

    if (generated_from_reset_weights == 0) {
        out << setw(16) << setprecision(3) << 0.0;
    } else {
        out << setw(16) << setprecision(3) << (100.0 * (double)inserted_from_reset_weights / (double)generated_from_reset_weights);
    }

    out << endl;

    out.close();

    out = fstream(output_directory + "/hyperparameters.txt", fstream::out | fstream::app);

    double min_initial_mu = 10, max_initial_mu = 0, avg_initial_mu = 0;
    double min_mu_delta = 10, max_mu_delta = 0, avg_mu_delta = 0;

    double min_initial_learning_rate = 10, max_initial_learning_rate = 0, avg_initial_learning_rate = 0;
    double min_learning_rate_delta = 10, max_learning_rate_delta = 0, avg_learning_rate_delta = 0;

    double min_initial_weight_decay = 10, max_initial_weight_decay = 0, avg_initial_weight_decay = 0;
    double min_weight_decay_delta = 10, max_weight_decay_delta = 0, avg_weight_decay_delta = 0;

    double min_hidden_dropout = 10, max_hidden_dropout = 0, avg_hidden_dropout = 0;
    double min_input_dropout = 10, max_input_dropout = 0, avg_input_dropout = 0;
    double min_velocity_reset = 10000000, max_velocity_reset = 0, avg_velocity_reset = 0;


    double best_initial_mu = genomes[0]->get_initial_mu();
    double best_mu_delta = genomes[0]->get_mu_delta();

    double best_initial_learning_rate = genomes[0]->get_initial_learning_rate();
    double best_learning_rate_delta = genomes[0]->get_learning_rate_delta();

    double best_initial_weight_decay = genomes[0]->get_initial_weight_decay();
    double best_weight_decay_delta = genomes[0]->get_weight_decay_delta();

    double best_hidden_dropout = genomes[0]->get_hidden_dropout_probability();
    double best_input_dropout = genomes[0]->get_input_dropout_probability();
    double best_velocity_reset = genomes[0]->get_velocity_reset();


    for (uint32_t i = 0; i < genomes.size(); i++) {
        if (genomes[i]->get_initial_mu() < min_initial_mu) {
            min_initial_mu = genomes[i]->get_initial_mu();
        }

        if (genomes[i]->get_initial_mu() > max_initial_mu) {
            max_initial_mu = genomes[i]->get_initial_mu();
        }
        avg_initial_mu += genomes[i]->get_initial_mu();

        if (genomes[i]->get_mu_delta() < min_mu_delta) {
            min_mu_delta = genomes[i]->get_mu_delta();
        }

        if (genomes[i]->get_mu_delta() > max_mu_delta) {
            max_mu_delta = genomes[i]->get_mu_delta();
        }
        avg_mu_delta += genomes[i]->get_mu_delta();


        if (genomes[i]->get_initial_learning_rate() < min_initial_learning_rate) {
            min_initial_learning_rate = genomes[i]->get_initial_learning_rate();
        }

        if (genomes[i]->get_initial_learning_rate() > max_initial_learning_rate) {
            max_initial_learning_rate = genomes[i]->get_initial_learning_rate();
        }
        avg_initial_learning_rate += genomes[i]->get_initial_learning_rate();

        if (genomes[i]->get_learning_rate_delta() < min_learning_rate_delta) {
            min_learning_rate_delta = genomes[i]->get_learning_rate_delta();
        }

        if (genomes[i]->get_learning_rate_delta() > max_learning_rate_delta) {
            max_learning_rate_delta = genomes[i]->get_learning_rate_delta();
        }
        avg_learning_rate_delta += genomes[i]->get_learning_rate_delta();


        if (genomes[i]->get_initial_weight_decay() < min_initial_weight_decay) {
            min_initial_weight_decay = genomes[i]->get_initial_weight_decay();
        }

        if (genomes[i]->get_initial_weight_decay() > max_initial_weight_decay) {
            max_initial_weight_decay = genomes[i]->get_initial_weight_decay();
        }
        avg_initial_weight_decay += genomes[i]->get_initial_weight_decay();

        if (genomes[i]->get_weight_decay_delta() < min_weight_decay_delta) {
            min_weight_decay_delta = genomes[i]->get_weight_decay_delta();
        }

        if (genomes[i]->get_weight_decay_delta() > max_weight_decay_delta) {
            max_weight_decay_delta = genomes[i]->get_weight_decay_delta();
        }
        avg_weight_decay_delta += genomes[i]->get_weight_decay_delta();


        if (genomes[i]->get_hidden_dropout_probability() < min_hidden_dropout) {
            min_hidden_dropout = genomes[i]->get_hidden_dropout_probability();
        }

        if (genomes[i]->get_hidden_dropout_probability() > max_hidden_dropout) {
            max_hidden_dropout = genomes[i]->get_hidden_dropout_probability();
        }
        avg_hidden_dropout += genomes[i]->get_hidden_dropout_probability();


        if (genomes[i]->get_input_dropout_probability() < min_input_dropout) {
            min_input_dropout = genomes[i]->get_input_dropout_probability();
        }

        if (genomes[i]->get_input_dropout_probability() > max_input_dropout) {
            max_input_dropout = genomes[i]->get_input_dropout_probability();
        }
        avg_input_dropout += genomes[i]->get_input_dropout_probability();


        if (genomes[i]->get_velocity_reset() < min_velocity_reset) {
            min_velocity_reset = genomes[i]->get_velocity_reset();
        }

        if (genomes[i]->get_velocity_reset() > max_velocity_reset) {
            max_velocity_reset = genomes[i]->get_velocity_reset();
        }
        avg_velocity_reset += genomes[i]->get_velocity_reset();
    }
    avg_initial_mu /= genomes.size();
    avg_mu_delta /= genomes.size();

    avg_initial_learning_rate /= genomes.size();
    avg_learning_rate_delta /= genomes.size();

    avg_initial_weight_decay /= genomes.size();
    avg_weight_decay_delta /= genomes.size();

    avg_hidden_dropout /= genomes.size();
    avg_input_dropout /= genomes.size();
    avg_velocity_reset /= genomes.size();

    out << setw(20) << setprecision(11) << min_initial_mu
        << setw(20) << setprecision(11) << max_initial_mu
        << setw(20) << setprecision(11) << avg_initial_mu
        << setw(20) << setprecision(11) << best_initial_mu
        << setw(20) << setprecision(11) << min_mu_delta
        << setw(20) << setprecision(11) << max_mu_delta
        << setw(20) << setprecision(11) << avg_mu_delta
        << setw(20) << setprecision(11) << best_mu_delta

        << setw(20) << setprecision(11) << min_initial_learning_rate
        << setw(20) << setprecision(11) << max_initial_learning_rate
        << setw(20) << setprecision(11) << avg_initial_learning_rate
        << setw(20) << setprecision(11) << best_initial_learning_rate
        << setw(20) << setprecision(11) << min_learning_rate_delta
        << setw(20) << setprecision(11) << max_learning_rate_delta
        << setw(20) << setprecision(11) << avg_learning_rate_delta
        << setw(20) << setprecision(11) << best_learning_rate_delta

        << setw(20) << setprecision(11) << min_initial_weight_decay
        << setw(20) << setprecision(11) << max_initial_weight_decay
        << setw(20) << setprecision(11) << avg_initial_weight_decay
        << setw(20) << setprecision(11) << best_initial_weight_decay
        << setw(20) << setprecision(11) << min_weight_decay_delta
        << setw(20) << setprecision(11) << max_weight_decay_delta
        << setw(20) << setprecision(11) << avg_weight_decay_delta
        << setw(20) << setprecision(11) << best_weight_decay_delta

        << setw(20) << setprecision(11) << min_input_dropout
        << setw(20) << setprecision(11) << max_input_dropout
        << setw(20) << setprecision(11) << avg_input_dropout
        << setw(20) << setprecision(11) << best_input_dropout

        << setw(20) << setprecision(11) << min_hidden_dropout
        << setw(20) << setprecision(11) << max_hidden_dropout
        << setw(20) << setprecision(11) << avg_hidden_dropout
        << setw(20) << setprecision(11) << best_hidden_dropout

        << setw(20) << setprecision(11) << min_velocity_reset
        << setw(20) << setprecision(11) << max_velocity_reset
        << setw(20) << setprecision(11) << avg_velocity_reset
        << setw(20) << setprecision(11) << best_velocity_reset
        << endl;
 
    out.close();
}

void EXACT::write_hyperparameters_header() {
    ifstream f(output_directory + "/hyperparameters.txt");
    if (f.good()) return;   //return if file already exists, don't need to rewrite header

    fstream out(output_directory + "/hyperparameters.txt", fstream::out | fstream::app);
    out << "# min initial mu"
        << ", max initial mu"
        << ", avg initial mu"
        << ", best initial mu"
        << ", min mu delta"
        << ", max mu delta"
        << ", avg mu delta"
        << ", best mu delta"

        << ", min initial learning rate"
        << ", max initial learning rate"
        << ", avg initial learning rate"
        << ", best initial learning rate"
        << ", min learning rate delta"
        << ", max learning rate delta"
        << ", avg learning rate delta"
        << ", best learning rate delta"

        << ", min initial weight decay"
        << ", max initial weight decay"
        << ", avg initial weight decay"
        << ", best initial weight decay"
        << ", min weight decay delta"
        << ", max weight decay delta"
        << ", avg weight decay delta"
        << ", best weight decay delta"

        << ", min input dropout"
        << ", max input dropout"
        << ", avg input dropout"
        << ", best input dropout"

        << ", min hidden dropout"
        << ", max hidden dropout"
        << ", avg hidden dropout"
        << ", best hidden dropout"

        << ", min velocity reset"
        << ", max velocity reset"
        << ", avg velocity reset"
        << ", best velocity reset"
        << endl;

    out.close();
}

void EXACT::write_statistics_header() {
    ifstream f(output_directory + "/progress.txt");
    if (f.good()) return;   //return if file already exists, don't need to rewrite header

    fstream out(output_directory + "/progress.txt", fstream::out | fstream::app);
    out << "# " << setw(14) << "generation id"
        << ", " << setw(14) << "new fitness"
        << ", " << setw(14) << "inserted"
        << ", " << setw(14) << "min_fitness"
        << ", " << setw(14) << "avg_fitness"
        << ", " << setw(14) << "max_fitness"
        << ", " << setw(14) << "min_epochs"
        << ", " << setw(14) << "avg_epochs"
        << ", " << setw(14) << "max_epochs"
        << ", " << setw(14) << "disable_edge"
        << ", " << setw(14) << "enable_edge"
        << ", " << setw(14) << "split_edge"
        << ", " << setw(14) << "add_edge"
        << ", " << setw(14) << "change_size"
        << ", " << setw(14) << "change_size_x"
        << ", " << setw(14) << "change_size_y"
        << ", " << setw(14) << "crossover"
        << ", " << setw(14) << "reset_weights"
        << endl;
    out.close();
}
