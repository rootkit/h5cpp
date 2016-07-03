#ifndef h5attr_GUARD_H
#define h5attr_GUARD_H

#include "hdf5.h"
#include <memory>
#include <string>
#include <map>
#include <vector>


class h5attr {
public:
    h5attr(std::string name, hid_t where, hid_t datatype,
            std::vector<hsize_t> dims);
    explicit h5attr(hid_t attr_id);

    void write(const void* data);

    ~h5attr();

private:
    std::string name;
    int drank;
    std::vector<hsize_t> dims;
    std::vector<hsize_t> max_dims;

    hid_t attr_id, dataspace_id;
    hid_t datatype;
    herr_t status;
};

#endif
