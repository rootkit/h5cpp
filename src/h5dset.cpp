#include "h5dset.h"
#include <iostream>

using namespace std;

dataspace::dataspace(vector<hsize_t> dims, vector<hsize_t> max_dims, 
                     vector<hsize_t> chunk_dims, bool compressed):
                dims(dims), max_dims(max_dims), chunk_dims(chunk_dims),
                compressed(compressed) {
    
    drank = dims.size(); 
    if (max_dims.empty())
        max_dims = dims;

    for (hsize_t i = 0; i != drank; i++) {
        if (max_dims[i] > dims[i])
            extendable = true;
        if (max_dims[i] == H5S_UNLIMITED) {
            unlimited = true;
            extendable = true;
        }
    }
    if (chunk_dims.size() > 0)
        chunked = true;
}


h5dset::h5dset(string name, hid_t where, hid_t datatype, dataspace dspace):
      name(name), datatype(datatype), dspace(dspace) {

    memspace = H5P_DEFAULT;
    prop = H5P_DEFAULT;
    if (dspace.extendable) {
        prop = H5Pcreate(H5P_DATASET_CREATE);
        status = H5Pset_chunk(prop, dspace.drank, dspace.chunk_dims.data());
    }

    dspace_id = H5Screate_simple(dspace.drank, dspace.dims.data(), 
                                 dspace.max_dims.data());
    dset_id = H5Dcreate2(where, name.c_str(), datatype,
                    dspace_id, H5P_DEFAULT, prop,
                    H5P_DEFAULT);
}

h5dset::h5dset(hid_t dset_id): dset_id(dset_id) {

    const int MAX_NAME = 1024; 
    char dset_name[MAX_NAME];

    H5Iget_name(dset_id, dset_name, MAX_NAME); 
    name = string(dset_name);

    dspace_id = H5Dget_space(dset_id);
    datatype = H5Dget_type(dset_id);
    prop = H5Dget_create_plist(dset_id);

    hsize_t drank = H5Sget_simple_extent_ndims(dspace_id);
    auto p_dims = make_unique<hsize_t[]>(drank);
    auto p_max_dims = make_unique<hsize_t[]>(drank);
    H5Sget_simple_extent_dims(dspace_id, p_dims.get(), p_max_dims.get());
    vector<hsize_t> dims(p_dims.get(), p_dims.get() + drank);
    vector<hsize_t> max_dims(p_max_dims.get(), p_max_dims.get() + drank);
    dspace = dataspace(dims, max_dims); 
}

unique_ptr<h5attr> h5dset::create_attribute(string name, hid_t datatype,
        vector<hsize_t> dims) {

    auto new_attr = make_unique<h5attr>(name, dset_id, datatype, dims);
    return new_attr;
}

void h5dset::extend(vector<hsize_t> size) {
    H5Dset_extent(dset_id, size.data());
    dspace.dims = size;
}

void h5dset::select(vector<hsize_t> offset, vector<hsize_t> count,
                    vector<hsize_t> stride, vector<hsize_t> block) {

    filespace = H5Dget_space(dset_id);
    status = H5Sselect_hyperslab(filespace, H5S_SELECT_SET, offset.data(),
            stride.data(), count.data(), block.data());
    memspace = H5Screate_simple(dspace.drank, count.data(), nullptr);
}

void h5dset::write(const void* data) {
    if (!dspace.extendable)
        status = H5Dwrite(dset_id, datatype, H5S_ALL, H5S_ALL,
               H5P_DEFAULT, data);
    else {
        status = H5Dwrite(dset_id, datatype, memspace, filespace, 
                     H5P_DEFAULT, data);
        H5Sclose(memspace);
        H5Sclose(filespace);
    }
}

void h5dset::append(const void* data) {
    vector<hsize_t> new_dims(dspace.dims);
    new_dims[dspace.drank-1] += 1;
    extend(new_dims);

    vector<hsize_t> offset(dspace.drank, 0);
    offset[dspace.drank-1] = new_dims[dspace.drank-1] - 1;
    vector<hsize_t> count(new_dims);
    count[dspace.drank-1] = 1;
    select(offset,count);

    write(data);
}

h5dset::~h5dset() {
    H5Pclose(prop);
    H5Dclose(dset_id);
} 
