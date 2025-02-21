#include <memory>
#include <map>
#include <numeric>
#include <sstream>
#include <fstream>
#include <string>
#include <algorithm>
#include <iostream>
#include "dataset_parser.h"

/* Parsing:
    - the data file needs to be comma separated
    - so far it only looks out for "?" as missing values, and then gets rid of those rows
    - you need to specify stuff like size, which features are numerical/categorical,
      which feature is the target and which features you want to drop.
    - cat_values is only relevant when using the grid. allows specifying how many values each 
      categorical feature may have. This allows hiding whether feature values are actually present in our dataset.

    Given these requirements, it should be easy to add new datasets in the same 
    fashion as the ones below. But make sure to double check what you get.
*/


DataSet *Parser::get_abalone(std::vector<ModelParams> &parameters,
        size_t num_samples, bool use_default_params)
{
    std::string file = "datasets/real/abalone.data";
    std::string name = "abalone";
    int num_rows = 4177;
    int num_cols = 8;
    std::shared_ptr<Regression> task(new Regression());
    std::vector<int> num_idx = {1,2,3,4,5,6,7};
    std::vector<int> cat_idx = {0};
    std::vector<int> target_idx = {8};
    std::vector<int> drop_idx = {};
    std::vector<int> cat_values = {}; // empty -> will be filled with the present values in the dataset

    DataSet *dset = parse_file(file, name, num_rows, num_cols, num_samples, task, num_idx,
        cat_idx, cat_values, target_idx, drop_idx, parameters, use_default_params);
    dset->cluster_ids = std::vector<int>(num_rows);
    return dset;
}

DataSet *Parser::get_adult(std::vector<ModelParams> &parameters,
        size_t num_samples, bool use_default_params)
{
    std::string file = "datasets/real/adult.data";
    std::string name = "adult";
    int num_rows = 48842;
    int num_cols = 14;
    std::shared_ptr<BinaryClassification> task(new BinaryClassification());
    std::vector<int> num_idx = {0,2,4,10,11,12}; //{0,4,10,11,12};
    std::vector<int> cat_idx = {1,3,5,6,7,8,9,13};
    std::vector<int> target_idx = {14};
    std::vector<int> drop_idx = {};//{2};
    std::vector<int> cat_values = {}; // empty -> will be filled with the present values in the dataset

    DataSet *dataset = parse_file(file, name, num_rows, num_cols, num_samples, task, num_idx,
        cat_idx, cat_values, target_idx, drop_idx, parameters, use_default_params);
    dataset->cluster_ids = std::vector<int>(num_rows);
    return dataset;
}

DataSet *Parser::get_spambase(std::vector<ModelParams> &parameters,
        size_t num_samples, bool use_default_params)
{
    std::string file = "datasets/real/spambase.data";
    std::string name = "spambase";
    int num_rows = 4601;
    int num_cols = 57;
    std::shared_ptr<BinaryClassification> task(new BinaryClassification());
    std::vector<int> num_idx = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56}; 
    std::vector<int> cat_idx = {};
    std::vector<int> target_idx = {57};
    std::vector<int> drop_idx = {};
    std::vector<int> cat_values = {}; // empty -> will be filled with the present values in the dataset

    DataSet *dataset = parse_file(file, name, num_rows, num_cols, num_samples, task, num_idx,
        cat_idx, cat_values, target_idx, drop_idx, parameters, use_default_params);

    for (int i=0; i<dataset->length; i++) {
        dataset->X[i][54] /= 10.0;
        dataset->X[i][55] /= 500.0;
        dataset->X[i][56] /= 5000.0;
    }

    return dataset;
}

/** Utility functions */

std::vector<std::string> Parser::split_string(const std::string &s, char delim)
{
    std::vector<std::string> result;
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        result.push_back(item);
    }
    return result;
}


DataSet *Parser::parse_file(std::string dataset_file, std::string dataset_name, int num_rows,
        int num_cols, int num_samples, std::shared_ptr<Task> task, std::vector<int> num_idx,
        std::vector<int> cat_idx, std::vector<int> cat_values, std::vector<int> target_idx, std::vector<int> drop_idx,
        std::vector<ModelParams> &parameters, bool use_default_params)
{
    std::ifstream infile(dataset_file);
    VVD X;
    std::vector<double> y;
    num_samples = std::min(num_samples, num_rows);

    if (use_default_params) {
        // create some default parameters
        ModelParams params = create_default_params();
        params.task = task;
        params.cat_idx = cat_idx;
        params.num_idx = num_idx;
        parameters.push_back(params);
    } else {
        // you have already defined your parameters, then just add dataset specific ones
        parameters.back().num_idx = num_idx;
        parameters.back().cat_idx = cat_idx;
        parameters.back().task = task;
        if(parameters.back().privacy_budget == 0){
            parameters.back().use_dp = false;
        }
    }

    std::string line;
    std::vector<std::string> whole_dataset;
    while (std::getline(infile, line,'\n')) {
        whole_dataset.push_back(line);
    }

    // shuffling deactivated here, this should happen in evaluation.cpp

    // parse dataset, label-encode categorical features
    int current_index = 0;
    std::vector<std::map<std::string,double>> mappings(num_cols + 1); // last (additional) one is for y

    for (auto line : whole_dataset) {
        
        if (current_index >= num_samples){
            break;
        }
        std::vector<std::string> strings = split_string(line, ',');
        std::vector<double> X_row;

        // drop dataset rows that contain missing entries ("?")
        if (line.find('?') < line.length() or line.empty()) {
            continue;
        }

        // go through each column
        for(size_t i=0; i<strings.size(); i++){

            // is it a drop column?
            if (std::find(drop_idx.begin(), drop_idx.end(), i) != drop_idx.end()) {
                continue;
            }

            // y
            if(std::find(target_idx.begin(), target_idx.end(), i) != target_idx.end()){
                if (dynamic_cast<Regression*>(task.get())) {
                    // regression -> y is numerical
                    y.push_back(stof(strings[i]));
                } else {
                    try { // categorical
                        double dummy_value = mappings.back().at(strings[i]);
                        y.push_back(dummy_value);
                    } catch (const std::out_of_range& oor) {
                        // new label encountered, create mapping
                        mappings.back().insert({strings[i], mappings.back().size()});
                        double dummy_value = mappings.back().at(strings[i]);
                        y.push_back(dummy_value);
                    }
                }
                continue;
            }

            // X
            if (std::find(num_idx.begin(), num_idx.end(), i) != num_idx.end()) {
                // numerical feature
                X_row.push_back(stof(strings[i]));
            } else {
                // categorical feature, do label-encoding
                try {
                    double dummy_value = mappings[i].at(strings[i]);
                    X_row.push_back(dummy_value);
                } catch (const std::out_of_range& oor) {
                    // new label encountered, create mapping
                    mappings[i].insert({strings[i], mappings[i].size()});
                    double dummy_value = mappings[i].at(strings[i]);
                    X_row.push_back(dummy_value);
                }
            }
        }
        X.push_back(X_row);
        current_index++;
    }

    // if we have more 1's than 0's switch the labels
    // otherwise our predict function assigns the wrong/opposite labels sometimes
    if(dynamic_cast<BinaryClassification*>(task.get())) {
        int count_zero = std::count(y.begin(), y.end(), 0.0);
        int count_one = std::count(y.begin(), y.end(), 1.0);
        if(count_one > count_zero){
            // switches 1 <-> 0
            std::transform(y.begin(),y.end(),y.begin(), [](double &d) { return 1.0 - d; } );
        }
    }

    DataSet *dataset = new DataSet(X,y);
    dataset->name = std::string(dataset_name) + std::string("_size_") + std::to_string(num_samples);

    // adjust cat_values if necessary
    if(cat_values.empty()) {
        for (size_t i=0; i<mappings.size()-1; ++i){
            // if feature is dropped, than do not add to cat_values
            if (std::find(drop_idx.cbegin(), drop_idx.cend(), i) != drop_idx.cend()) continue;

            auto map = mappings[i];
            std::vector<double> keys;
            if (std::find(cat_idx.cbegin(), cat_idx.cend(), i) == cat_idx.cend()) {
                parameters.back().cat_values.push_back(keys);
                continue;
            }
            for(auto it = map.begin(); it != map.end(); ++it){
                keys.push_back(it->second);
            }
            std::sort(keys.begin(), keys.end());
            parameters.back().cat_values.push_back(keys);
        }
    } else {
        for(int i=0; i<num_cols; ++i) {
            // if feature is dropped, than do not add to cat_values
            if (std::find(drop_idx.cbegin(), drop_idx.cend(), i) != drop_idx.cend()) continue;

            std::vector<double> keys;
            if (std::find(cat_idx.cbegin(), cat_idx.cend(), i) == cat_idx.cend()){
                parameters.back().cat_values.push_back(keys);
                continue;
            }
            for(double j=0.0; j<cat_values[i]; ++j){
                keys.push_back(j);
            }
            std::sort(keys.begin(), keys.end());
            parameters.back().cat_values.push_back(keys);
        }
    }

    // update num_idx / cat_idx if we dropped columns
    for(auto drop_elem : drop_idx) {
        for(auto &num_elem : parameters.back().num_idx){
            num_elem = num_elem > drop_elem ? num_elem - 1 : num_elem;
        }
        for(auto &cat_elem : parameters.back().cat_idx){
            cat_elem = cat_elem > drop_elem ? cat_elem - 1 : cat_elem;
        }
    }

    return dataset;
}
