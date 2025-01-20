#include <pybind11/pybind11.h>
// extern "C" {
// #include <rad_logger.h> // Include the C library header
// }

#include <rad_logger.h>

namespace py = pybind11;

PYBIND11_MODULE(rad_logger, m)
{
	m.doc() = "Python bindings for the rad_logger library"; // Module documentation

	// Expose the log_start function
	m.def(
		"log_start",
		[](int event_id, const std::string &event_name, const std::string &event_type,
		   const std::string &event_data) -> int {
			// Call the C function
			return log_start(event_id, event_name.c_str(), event_type.c_str(),
					 event_data.c_str());
		},
		py::arg("event_id"), // Argument: int
		py::arg("event_name"), // Argument: const char* (std::string in Python)
		py::arg("event_type"), // Argument: const char* (std::string in Python)
		py::arg("event_data"), // Argument: const char* (std::string in Python)
		"Start logging with the specified event details.\n\n"
		"Parameters:\n"
		"  event_id (int): The ID of the event.\n"
		"  event_name (str): The name of the event.\n"
		"  event_type (str): The type of the event.\n"
		"  event_data (str): Additional data about the event.\n\n"
		"Returns:\n"
		"  int: The result of the logging operation.");
}
