# S-BDT
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.12783789.svg)](https://doi.org/10.5281/zenodo.12783789)

[S-BDT: Distributed Differentially Private Boosted Decision Trees](https://arxiv.org/abs/2309.12041)

by Thorsten Peinemann\*, Moritz Kirschte\*, Joshua Stock, Carlos Cotrini, Esfandiar Mohammadi.
arXiv, Sep. 2023.

\* The first two authors equally contributed to this work.

This repository is based on the [work](https://github.com/loretanr/dp-gbdt) by Rudolf Loretan.


Table of Contents:
- [S-BDT](#s-bdt)
  - [Requirements](#requirements)
  - [Installing](#installing)
    - [C++ Library](#c-library)
    - [CPython Module](#cpython-module)
  - [Reproducing our results](#reproducing-our-results)
    - [Python Variant](#python-variant)
      - [C++ Variant](#c-variant)
  - [Including S-BDT into your own projects](#including-s-bdt-into-your-own-projects)
    - [Other CMake Projects](#other-cmake-projects)
    - [Python / Jupyter Notebook](#python--jupyter-notebook)
  - [Documentation Reference](#documentation-reference)
    - [Parameter Specification](#parameter-specification)
    - [Privacy Accounting](#privacy-accounting)
  - [Datasets](#datasets)
    - [Custom Dataset](#custom-dataset)
    - [Data splits for a stream of non-IID data](#data-splits-for-a-stream-of-non-iid-data)
  - [Limitations](#limitations)



## Requirements

tested on a Ubuntu 22.04 with Python 3.10.12 and g++ 11.4.0.
We wrote the code in C++11 and provided a Cython-based Python wrapper.

C++ code only:
```bash
sudo apt install libspdlog-dev icdiff libfmt-dev ninja-build -y
```

Additionally, for the Cython wrapper:
```bash
sudo apt install python3-pip
sudo pip3 install icc_rt  # install libsvml globally
sudo ldconfig  # add libsvml to shared libraries
```

## Installing

### C++ Library

After installing all dependencies, you can build using CMake:

```bash
mkdir build && cd build

cmake -DNinja ..

cmake --build .
sudo cmake --install .
```

By default this will install `libsbdt.so` to `/usr/local/lib` and all headers to `/usr/local/include`. You can change the install path by setting `CMAKE_INSTALL_PREFIX`.

```bash
# example /usr will install to /usr/lib and /usr/include
sudo cmake --install -DCMAKE_INSTALL_PREFIX=<path> .
```

### CPython Module

**NOTE: you have to build and install the C++ library before building the python module! See [C++ Library section](#c-library) for more info.**

Optional: Create a python [virtual environment for better isolation](https://docs.python.org/3/library/venv.html) before installing:

```bash
cd <project root directory>

python -m venv .venv
source .venv/bin/activate
```

Build and install the S-BDT python module:

```bash
cd code 

python -m pip install -r requirements.txt # install requirements
python -m pip install . # install S-BDT
```

If you installed the C++ Library to a custom path (i.e. not to `/usr/local`), you have can simply set `CMAKE_INSTALL_PREFIX` before calling pip. Example:

```sh
export CMAKE_INSTALL_PREFIX="/usr/"
python -m pip install .
```

## Reproducing our results

### Python Variant

We refer to [example_pycall.py](/code/example_pycall.py) for the configuration of a single run.
We note that we provide a sklearn-like interface that works with NumPy arrays:

```python
import sbdt
model = sbdt.S_BDT(/*params*/)
model.train(X_train, y_train)
y_pred = model.predict(X_test)
score = model.score(y_test, y_pred, /*scoring function enum*/)
```

Build:

Follow the steps outlaid in the [CPython Module section](#cpython-module)pycall.py

Run:

```bash
cd code

python3 example_pycall.py
```

The script will load the [Abalone dataset](https://archive.ics.uci.edu/dataset/1/abalone), train with predefined parameters (`ε = 0.1`) and render the first 20 trees into `code/img`.

#### C++ Variant

We refer to [main.cpp](code/cpp_gbdt/src/main.cpp) for the configuration of a single non-multithreaded cross-validated run and [evaluation.cpp](code/cpp_gbdt/src/evaluation.cpp) for a multi-threaded hyperparameter search template that creates a .csv in the [results](code/cpp_gbdt/results) directory.

Build:

By default, the c++ example application **won't be build**. You have to set `BUILD_EXAMPLE_APPLICATION` to `ON`. This will also download the required datasets.

```bash
mkdir build && cd build

cmake -DNinja -DBUILD_EXAMPLE_APPLICATION=ON ..

cmake --build .
```

Run:

```bash
# IMPORTANT: Make sure you are in the same directory as the downloaded datasets
# .
# ├── code
# │   └── cpp_gbdt
# │       └── run_example
# └── datasets
#     └── real
#         ├── abalone.data
#         ├── adult.data
#         └── spambase.data

cd build

./code/cpp_gbdt/run_example
```

## Including S-BDT into your own projects

### Other CMake Projects

After [installing](#c-library), S-BDT registers itself as a CMake project. This means, that you are simply able to include it as you would any other package. See [main.cpp](code/cpp_gbdt/src/main.cpp) for an example `main` file.

```cmake
cmake_minimum_required(VERSION 3.5.0)
project(sbdt-example VERSION 0.1.0 LANGUAGES C CXX)

# create target
set(TARGET sbdt-train)
add_executable(${TARGET} main.cpp)

# import S-BDT package
find_package(sbdt REQUIRED)

# link it to your target
target_link_libraries(${TARGET} PUBLIC sbdt::sbdt)
```

### Python / Jupyter Notebook

After installing S-BDT with pip, your can use it as any other python module. See [example_pycall.py](/code/example_pycall.py) for an example on how you could use S-BDT.

```py
import sbdt
...
```

When using Jupyter Notebooks: Depending on your setup, you might need to build S-BDT directly inside Jupyter. You can include the following magic command at the top of your notebook.

```bash
%pip install "<repository root>/code/"

# OR

!python -m pip install "<repository root>/code"
```


## Documentation Reference

### Parameter Specification

Parameters as specified in [parameters.h](code/cpp_gbdt/include/parameters.h) and similarly in the Python wrapper:
- **tree ensemble**
  - int nb_trees. default: 150. *The number of trees to train.*
  - double subsampling_ratio. Default: 0.1. *The Poisson subsampling rate which decides how many data points are used within each tree.*
  - double learning_rate. Default: 0.1. *The learning rate of the BDT algorithm.*
  - int max_depth. Default: 2. *The tree depth. For disabled DP and scoring-based splits, this is the maximal tree depth.*
  - bool newton_boosting. Default: false. *Whether to use Newton Boosting, i.e. approximating the leaf denominator with the Hessian. If deactivated for binary classification, it switches the gradients/Hessians to the MSE loss which has a constant Hessian of 1. For regression, this flag has no effect.* 
  - bool balance_partition. Default: true. *Decides whether we balance the data point per tree equally. Deactivating this is only relevant for Laplace noise where we do not apply subsampling. Then the selection procedure is based on line 8 of Algorithm 2 in Li et al. (AAAI-20).*
  - lambda_reg_type lambda_reg_mode. Default: ADD. *Possible values: MAX or AND. Differentiates the leaf denominator: for MAX, we have `max(lambda, sum(hessian_i))`; for ADD, we have `lambda + sum(hessian_i)`.*
  - double l2_lambda. Default: 15. *L2-Regularization parameter of the BDT objective function. To avoid divisions by zero, this lambda should be strictly larger than 0.*
  - double reg_delta. Default: 2. *Regularization parameter for the leaf prediction. It clamps the leaf prediction as well as the noised leaf prediction by reg_delta to prevent exploding leaf predictions. Setting reg_delta to 0 deactivates this regularization.*
- **splits**
  - int min_samples_split. Default: 0. *The minimal number of data points within a split to continue splitting, otherwise a leaf is built. It should be set to 0 for DP.*
  - bool ignore_split_constraints. Default: true. *For discrete random split candidates (cf. Maddock et al. (CCS-22)), activating this flag enforces that splits are always reachable and halts the splitting early if no such split candidate is available.*
  - max_feature_type max_features. Default: RAND. *Possible values: RAND, ONE, LOG2, SQRT, ALL. Variants of how the features for splitting are chosen. Only active if cyclical_feature_interactions is deactivated. ONE has the same behavior as RAND where both uniformly at random select one of the available feature indices. LOG2 selects log2 many features, SQRT selects sqrt many features, and ALL selects all features. The latter three or all variants for max_feature_values != RAND require a privacy budget for the split: privacy_budget_gain_ratio > 0. The former two variants overwrite privacy_budget_gain_ratio = 0 if max_feature_values=RAND.*
  - max_feature_type max_feature_values. Default: RAND. *Possible values: RAND, ONE, LOG2, SQRT, ALL. Variants of how the feature values for splitting are chosen. RAND selects a split value uniformly at random (or between the split candidates if random_splits_from_candidates is activated) in the feature_val_border range (numerical feature) or between the cat_values (categorical feature). ONE selects one feature value, LOG2 selects log2 many feature values, SQRT selects sqrt many feature values, and ALL selects all feature values. Note that in the latter cases and for a numerical feature, a dummy feature value at the borders of feature_val_border is inserted and all values outside the borders are cut away. privacy_budget_gain_ratio is affected as documented for the max_features parameter above.*
  - criterion_type criterion. Default: XGD_MSE. *Possible values: XGD_MSE, XGBOOST. It decides which split criterion to use which is only relevant for non-random splits. Note that XGBOOST only works without privacy as in the code no sensitivity is set.*
  - bool reuse_attr. Default: true. *Whether to reuse feature indices in a subtree that has been previously split on. If deactivated, the number of features naturally limits the split depth. Deactivated if cyclical_feature_interactions is activated.*
- **privacy**
  -  bool use_dp. Default: true. *Whether to train with or without differential privacy.*
  - double privacy_budget. Default: 0.1. *The target privacy budget (epsilon). Note that the accounted privacy budget will be slightly off (details below).*
  - double privacy_budget_init_score_ratio. Default: 0.1. *The ratio of the privacy budget spent to calculate the initial score (for regression: the mean) of BDTs differentially private.*
  - double privacy_budget_gain_ratio. Default: 0.5. *The ratio of the privacy budget that will be spent on the scoring function for the split. Only active if using a scoring function, i.e. either `max_features` or `max_feature_values` is set to `LOG2, SQRT, ALL`.*
  - double leaf_denom_noise_weight. Default: 0.2. *The ratio of the privacy budget spent on approximating the leaf denominator i.e. the Hessian.*
  - double l2_threshold. Default: 0.1. *The l2-norm-clipping threshold for each gradient.*
  - double hess_l2_threshold. Default: 1.0. *The l2-norm-clipping threshold for each hessian.*
  - double init_score_threshold. Default: 1.0. *The l2-norm-clipping threshold for each summand in the init score.*
  - double numeric_feature_weight. Default: 1.0. *Weights the numerical features above the categorical ones when a DP split selection is enabled. The number of possible categorical values weights each categorical feature.*
  - noise_type leaf_noise. Default: GAUSS. *Whether to use Gauss or Laplace noise. Note that, for Laplace noise, we deactivate features like the individual Rényi filter, RDP accounting, and subsampling. For Gauss noise, we deactivate features like balance partition and gradient filtering.*
- **individual privacy filter**
  - bool use_privacy_filter. Default: false. *Whether to train with or without the individual Rényi filter.*
  - bool approximate_privacy_filter. Default: false. *For a performance boost on privacy accounting, activate this flag. However, it will only approximate the DP guarantees and should thus be used with care.*
  - int int pf_additional_nb_trees. Default: 0. *The number of extra trees to train that get privacy for free due to individual Réyni filters.*
  - double pf_l2_threshold. Default: 0.1. *The l2-norm-clipping threshold for each gradient in the extra rounds of the individual Rényi filter.*
  - double pf_hess_l2_threshold. Default: 1.0. *The l2-norm-clipping threshold for each hessian in the extra rounds of the individual Rényi filter.*
  - double pf_subsampling_ratio_factor. Default: 1.0. *The Poisson subsampling multiplier in the extra rounds of the individual Rényi filter. A value of 1 means that the same subsampling rate is used in all training rounds.*
- **stream**
  - int additional_nb_trees. Default: 0. *The number of trees added at the end of regular training to accommodate for newly arrived data.* 
- **other configs**
  - int refine_splits_rounds. Default: 0. *Determines how many rounds the splits are optimized as introduced by Maddock et al. (CCS-22). Starting from this tree index, split refinement is deactivated. Each additional round increases the privacy consumption due to sequential composition.*
  - int num_split_candidates. Default: 32. *Determines the initial number of split candidates in the root of each tree. From these are the splits chosen.*
  - bool gradient_filtering. Default: false. *Whether to use gradient-based Data Filtering (GDF) as introduced by Li et al. (AAAI-20). Here, we filter data points instead of clipping them. Note that, with GDF, data points with gradients always below the `l2_threshold` are not used during training.*
  - bool leaf_clipping. Default: false. *Whether to use geometric leaf clipping (GLC) as introduced by Li et al. (AAAI-20). Here, the leaves are clipped with an l2_threshold that falls geometrically with the number of trees.*
  - bool cyclical_feature_interactions. Default: true. *Whether to activate the cyclical feature interaction as introduced by Nori et al. (ICML-21). Here, each tree only splits on one feature index. It overwrites reuse_attr and max_features.*
  - bool refine_splits. Default: false. *Whether to activate the differentially private split refinement procedure "iterative Hessian" as introduced by Maddock et al. (CCS-22). Here, we refine (i.e. merge and subdivide) an initial equidistant split candidate set during ensemble training using the aggregated Hessian for each split candidate. It is only possible if the individual Rényi filter is deactivated and random_splits_from_candidates is activated.*
  - bool random_splits_from_candidates. Default: true. *Whether to choose a random split uniformly at random from a candidate set (true) or uniformly at random from the feature range (false). *
  - double refine_splits_subsample. Default: 1.0.  *The subsampling rate of the data points chosen for split refinement.*
  - bool cut_off_leaf_denom. Default: true. *Sets the prediction of a leaf to 0 if activated and the denominator is negative.*
- **debugging**
  - double custom_noise_scale. Default: -1. *If larger than 0, this noise scale will be used in noising and accounting instead of being optimized to the privacy_budget target.*
  - int verbosity. Default: 4; *Only relevant for the Cython wrapper: 1: debug; 2: info; 4: err. For C++, set the log level directly in the main.cpp or evaluation.cpp.* 
- **dataset**
  - std::shared_ptr<Task> task. *Whether to use Binary Classification or Regression. Each has different loss, gradient, hessian, and scoring functions.*
  - std::pair<double, double> feature_val_border. Default: {0., .5}. *The lower and upper border of the feature value range from which the splits are chosen. The same range applies to all features equally.*
  - bool continuous_learning. Default: false. *Activate for learning on a stream of non-IID data. Currently, it only supports 'adult' and 'abalone' datasets identified via the `DataSet.name` attribute (details below).*
  - bool scale_y. Default: true. *Whether to min-max-scale the regression values for the BDT training which primarily influences the range of valid l2_thresholds to a fixed range. This might be helpful for hyperparameter tuning. Note that for C++ this feature has to be done manually e.g. as used in the `code/cpp_gbdt/src/main.cpp` file.*
  - std::vector<int> cat_idx. *cf. below in "Custom Dataset".*
  - std::vector<int> num_idx. *cf. below in "Custom Dataset".*
  - std::vector<std::vector<double>> cat_values. *cf. below in "Custom Dataset".*

Compile time parameters (Makros):
Can be specified in the [Makefile](code/cpp_gbdt/Makefile) using the `MAKROFLAG` or in [parameters.h](code/cpp_gbdt/include/parameters.h).
- DELTA. Default: 5e-8. $\delta$ *of differential privacy.*
- MAX_ALPHA. Default: 2000. *The maximal possible* $\alpha$ *value of RDP accounting. Smaller values improve the performance. Larger values may improve the privacy-utility tradeoff in the small epsilon regime, e.g.* $\varepsilon \leq 0.1$.

### Privacy Accounting

We utilize Rényi differential privacy (RDP) accounting and export the accounted `real_eps`, the $\rho(\alpha)$ divergence `max_rho`, the moment `alpha`, and the noise scale of each leaf `leaf_sigma` to the BDT model.

## Datasets

We support the following pre-defined datasets as well as custom datasets via our Cython wrapper. To use the pre-defined datasets in C++, download the datasets under the following link into the `datasets/real` directory (relative to the working directory).

- [Abalone](https://archive.ics.uci.edu/dataset/1/abalone), regression (4,177 rows, 8 features)
- [Adult](https://archive.ics.uci.edu/dataset/2/adult), binary classification (48,842 rows, 14 features)
- [Spambase](https://archive.ics.uci.edu/dataset/94/spambase), binary classification (4,601 rows, 57 features)

As evaluation metrics for the score-routine, we support for regression the root mean squared error (RMSE=0) and for binary classification the threshold-tuned accuracy (ACC=1), untuned accuracy (UNTUNED_ACC=2), a fast area under the ROC curve approximation (AUC_WMW=3), the exact AUC (AUC=4), and the F1 score (F1=5).

### Custom Dataset

As shown in [example_pycall.py](code/example_pycall.py), the Cython wrapper supports custom datasets: it expects a dataset `x` with labels `y` and optionally a dataset `name` and `cluster_ids` as input to the train-routine and some hyperparameters `cat_idx`, `num_idx`, `cat_values`. The parameter and input descriptions are the same as for C++ and are described below.

For the C++ code, add a new CSV-formatted dataset in the code at [dataset_parser.cpp](code/cpp_gbdt/src/dataset_parser.cpp#L24). In particular, we require the following details:

-    std::string file. *File path.*
-    std::string name. *Choose your dataset name. For a stream of non-IID data only: the values "abalone" and "adult" use pre-defined data splits and the value "custom" uses custom data splits.*
-    int num_rows. *Dataset size.*
-    int num_cols. *Number of features.*
-    std::shared_ptr<Regression> task. *The task: `new Regression()` or `new BinaryClassification()`.*
-    std::vector<int> num_idx. *List of indices that are numerical features.*
-    std::vector<int> cat_idx. *List of indices that are categorical features.*
-    std::vector<int> target_idx. *List which only contains one feature index that represents the label or regression target.*
-    std::vector<int> drop_idx. *Drop some feature indices here.*
-    std::vector<int> cat_values. *Set manually only for Cython: List of lists of all possible categorical features. It contains an empty list at the feature index that is non-categorical and a list of all possible features at all other positions.*
-    std::vector<int> cluster_ids. *For a stream of non-IID data only: A `num_rows`-large List where "0" assigns a data point to the initial dataset and "1" to the hold-back added-later dataset.*

### Data splits for a stream of non-IID data

S-GDT is amenable to learning on a stream of non-IID data. This is simulated by holding back parts of training data and then later adding them back into the training dataset. Hence, we split the dataset into an initial dataset (available for at least the first `nb_tree` many rounds) and a hold-back added-later dataset (available after `nb_tree` many rounds, hence only for `additional_nb_trees` or `pf_additional_nb_trees` many extra rounds).

Usage for the Cython wrapper train-routine (call: `model.train(X, y, name, cluster_ids)`): Either use the predefined data split by setting a dataset `name` that matches in a substring with "abalone" or "adult" (cf. [dp_ensemble.cpp](code/cpp_gbdt/src/gbdt/dp_ensemble.cpp#L157)) or provide a custom data split by setting `name=custom` and input `cluster_ids`. For C++, loading the data automatically sets the correct dataset name.

The pre-defined data splits can be altered at [dp_ensemble.cpp](code/cpp_gbdt/src/gbdt/dp_ensemble.cpp#L155) and are configured as follows:
- For Abalone, all data points with a regression value larger than the mean (mean is -0.36, after scaling the regression value to [-1,1]) are held back initially.
- For Adult, all data points with label == 1 are held back initially.

## Limitations

- Our implementation only focuses on **regression** and **binary classification**.
  - While the Python variant also works for custom datasets, the C++ variant only supports the abalone, adult, and spambase datasets.
- Training with Laplace noise only supports a single ensemble i.e. each data point is at most used once.
- Constraining the splits to be only performed within the current splitting interval of a given subtree (configure via *ignore_split_constraints*) only works with split candidates for random splits (set via *random_splits_from_candidates=true*).
- Refinement of splits (set via *refine_splits=true*, see iterative Hessian in Maddock et al., https://arxiv.org/pdf/2210.02910) does not work together with individual privacy accounting.
