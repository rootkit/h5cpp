#ifndef h5file_GUARD_H
#define h5file_GUARD_H

#include "hdf5.h"
#include <memory>
#include "boost/multi_array.hpp"
#include <string>
#include <vector>
#include <map>

#include "h5group.h"
#include "h5dset.h"
#include "h5attr.h"


namespace h5cpp {

enum class io {
    w,       ///<  over-write
    wn,      ///<  write new; fail if file already exists
    wp,      ///<  create if non-existant; otherwise open
    r,       ///<  read only
    rw       ///<  read-write
};

class h5file {
public:
    h5file();
    h5file(std::string name, io flag, hid_t prop = H5Pcreate(H5P_FILE_ACCESS));
    h5file& operator=(const h5file& rhs); 

    h5group create_group(std::string name);
    h5dset create_dataset(std::string name, dtype datatype, dspace dataspace);
    h5dset create_dataset(std::string name, hid_t datatype, dspace dataspace);
    h5dset create_dataset(std::string name, dtypeCompound datatype, dspace dataspace);
    h5attr create_attribute(std::string name, dtype datatype, dspace dataspace = dspace());

    h5group open_group(std::string name);
    h5dset open_dataset(std::string name);
    h5attr open_attribute(std::string name, std::string base="/");

    h5group create_or_open_group(std::string name);
    h5dset create_or_open_dataset(std::string name, dtype datatype, dspace dataspace);

    bool object_exists(std::string name);

    ~h5file();

private:
    std::string filename;
    hid_t file_id;
    hid_t prop_id;
    herr_t status;
};

}

#endif
