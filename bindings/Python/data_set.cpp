#include "plssvm/data_set.hpp"

#include "utility.hpp"  // check_kwargs_for_correctness

#include "fmt/core.h"           // fmt::format
#include "fmt/format.h"         // fmt::join
#include "pybind11/pybind11.h"  // py::module_, py::class_, py::init, py::return_value_policy, py::arg, py::kwargs, py::value_error, py::pos_only
#include "pybind11/stl.h"       // support for STL types

#include <string>   // std::string
#include <utility>  // std::move
#include <vector>   // std::vector

namespace py = pybind11;
using namespace std::literals;

void init_data_set(py::module_ &m) {
    // bind data_set class
    using real_type = double;
    using label_type = std::string;
    using data_set_type = plssvm::data_set<real_type, label_type>;
    using size_type = typename data_set_type::size_type;

    // bind the plssvm::data_set::scaling internal "factors" struct
    py::class_<data_set_type::scaling::factors>(m, "data_set_scaling_factors")
        .def(py::init<size_type, real_type, real_type>(), py::arg("feature"), py::arg("lower"), py::arg("upper"))
        .def_readonly("feature", &data_set_type::scaling::factors::feature)
        .def_readonly("lower", &data_set_type::scaling::factors::lower)
        .def_readonly("upper", &data_set_type::scaling::factors::upper)
        .def("__repr__", [](const data_set_type::scaling::factors &scaling_factors) {
            return fmt::format("<plssvm.data_set.scaling.factors with {{ feature: {}, lower: {}, upper: {} }}>",
                               scaling_factors.feature,
                               scaling_factors.lower,
                               scaling_factors.upper);
        });

    // bind the plssvm::data_set internal "scaling" struct
    py::class_<data_set_type::scaling>(m, "data_set_scaling")
        .def(py::init<real_type, real_type>(), py::arg("lower"), py::arg("upper"))
        .def(py::init<const std::string &>())
        .def("save", &data_set_type::scaling::save)
        .def_readonly("scaling_interval", &data_set_type::scaling::scaling_interval)
        .def_readonly("scaling_factors", &data_set_type::scaling::scaling_factors)
        .def("__repr__", [](const data_set_type::scaling &scaling) {
            return fmt::format("<plssvm.data_set.scaling with {{ lower: {}, upper: {}, #factors: {} }}>",
                               scaling.scaling_interval.first,
                               scaling.scaling_interval.second,
                               scaling.scaling_factors.size());
        });

    // bind the data set class
    py::class_<data_set_type>(m, "data_set")
        .def(py::init([](const std::string &file_name, py::kwargs args) {
            // check for valid keys
            check_kwargs_for_correctness(args, { "file_format"sv, "scaling"sv });

            // call the constructor corresponding to the provided named arguments
            if (args.contains("file_format") && args.contains("scaling")) {
                return data_set_type{ file_name, args["file_format"].cast<plssvm::file_format_type>(), args["scaling"].cast<data_set_type::scaling>() };
            } else if (args.contains("file_format")) {
                return data_set_type{ file_name, args["file_format"].cast<plssvm::file_format_type>() };
            } else if (args.contains("scaling")) {
                return data_set_type{ file_name, args["scaling"].cast<data_set_type::scaling>() };
            } else {
                return data_set_type{ file_name };
            }
        }))
        .def(py::init([](std::vector<std::vector<real_type>> data, py::list labels, py::kwargs args) {
            // check named arguments
            check_kwargs_for_correctness(args, { "scaling" });

            // TODO: investigate performance implications?
            std::vector<std::string> tmp(py::len(labels));
            #pragma omp parallel for
            for (std::vector<std::string>::size_type i = 0; i < py::len(labels); ++i) {
                tmp[i] = labels[i].cast<py::str>().cast<std::string>();
            }

            if (args.contains("scaling")) {
                return data_set_type{ std::move(data), std::move(tmp), args["scaling"].cast<data_set_type::scaling>() };
            } else {
                return data_set_type{ std::move(data), std::move(tmp) };
            }
        }))
        .def(py::init([](std::vector<std::vector<real_type>> data, py::kwargs args) {
            // check named arguments
            check_kwargs_for_correctness(args, { "scaling" });

            if (args.contains("scaling")) {
                return data_set_type{ std::move(data), args["scaling"].cast<data_set_type::scaling>() };
            } else {
                return data_set_type{ std::move(data) };
            }
        }))
        .def("save", &data_set_type::save)
        .def("data", &data_set_type::data, py::return_value_policy::reference_internal)
        .def("has_labels", &data_set_type::has_labels)
        .def("labels", &data_set_type::labels, py::return_value_policy::reference_internal)
        .def("different_labels", &data_set_type::different_labels)
        .def("num_data_points", &data_set_type::num_data_points)
        .def("num_features", &data_set_type::num_features)
        .def("num_different_labels", &data_set_type::num_different_labels)
        .def("is_scaled", &data_set_type::is_scaled)
        .def("scaling_factors", &data_set_type::scaling_factors, py::return_value_policy::reference_internal)
        .def("__repr__", [](const data_set_type &data) {
            std::string optional_repr{};
            if (data.has_labels()) {
                optional_repr += fmt::format(", labels: [{}]", fmt::join(data.different_labels().value(), ", "));
            }
            if (data.is_scaled()) {
                optional_repr += fmt::format(", scaling: [{}, {}]",
                                             data.scaling_factors()->get().scaling_interval.first,
                                             data.scaling_factors()->get().scaling_interval.second);
            }
            return fmt::format("<plssvm.data_set with {{ #points: {}, #features: {}{} }}>",
                               data.num_data_points(),
                               data.num_features(),
                               optional_repr);
        });
}