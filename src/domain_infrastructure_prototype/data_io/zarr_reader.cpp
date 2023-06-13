#include "zarr_reader.h"
using namespace std;

template<typename T>
bool decompress_zstd_file(const string& filename, T** decompressed_data, size_t& decompressed_size)
{
    ifstream file(filename, ios::binary);
    if (!file.is_open())
    {
        cout << "Error opening file: " << filename << endl;
        return false;
    }

    // Get the compressed data size from the file
    file.seekg(0, ios::end);
    size_t compressed_size = file.tellg();
    file.seekg(0, ios::beg);
    if (file.fail())
    {
        cout << "Error reading data size from file: " << filename << endl;
        file.close();
        return false;
    }

    // Allocate memory for the compressed data
    char* compressed_data = new char[compressed_size];

    file.read(compressed_data, compressed_size);
    if (file.fail())
    {
        cout << "Error reading compressed data from file: " << filename << endl;
        delete[] compressed_data;
        file.close();
        return false;
    }

    // Get the decompressed size
    decompressed_size = ZSTD_getFrameContentSize(compressed_data, compressed_size);
    if (decompressed_size == ZSTD_CONTENTSIZE_ERROR)
    {
        cout << "Error getting decompressed size" << endl;
        delete[] compressed_data;
        file.close();
        return false;
    }

    // Allocate memory for the decompressed data
    *decompressed_data = new T[decompressed_size / sizeof(T)];

    // Decompress the data
    size_t result = ZSTD_decompress(*decompressed_data, decompressed_size, compressed_data, compressed_size);

    // Clean up
    delete[] compressed_data;
    file.close();

    if (ZSTD_isError(result))
    {
        cout << "Error decompressing data: " << ZSTD_getErrorName(result) << endl;
        delete[] *decompressed_data;
        return false;
    }

    return true;
}

void open_no_dask_zarr(complex<double>** vis, double** weight,  double** uvw, double** chan, long** vis_chunk_shape, const string& zarr_name, int time_chunk_id, int chan_chunk_id){
    //Read metadata
    //.zarray
    ifstream vis_meta_stream(zarr_name+"/DATA/.zarray", ifstream::in);
    json vis_meta = json::parse(vis_meta_stream);
    
    //cout << vis_meta["chunks"].size() << endl;
    //int vis_chunk_shape[vis_meta["chunks"].size()];
    *vis_chunk_shape = new long[vis_meta["chunks"].size()];
    //cout << vis_meta["chunks"].size() << endl;
    
    int s;
    for(int i=0; i < vis_meta["chunks"].size(); i++){
        long s = static_cast<long>(vis_meta["chunks"][i]);
        (*vis_chunk_shape)[i] = s;
        //cout << "vis_data_meta " << vis_chunk_shape[i] << " ,*, " << typeid(vis_chunk_shape[i]).name() << endl;
    }
    
    //time,baseline,chan,pol
    //cout << zarr_name+"/DATA/"+to_string(time_chunk_id)+".0."+to_string(chan_chunk_id)+".0" << endl;
    
    auto start = chrono::high_resolution_clock::now();
    //complex<double>* vis;
    string filename = zarr_name+"/DATA/"+to_string(time_chunk_id)+".0."+to_string(chan_chunk_id)+".0";
    size_t vis_size;
    decompress_zstd_file(filename, vis, vis_size);
    
    //float* weight;
    filename = zarr_name+"/WEIGHT/"+to_string(time_chunk_id)+".0."+to_string(chan_chunk_id)+".0";
    size_t weight_size;
    decompress_zstd_file(filename, weight, weight_size);
    
    //float* uvw;
    filename = zarr_name+"/UVW/"+to_string(time_chunk_id)+".0.0";
    size_t uvw_size;
    decompress_zstd_file(filename, uvw, uvw_size);
    
    //chan
    filename = zarr_name+"/chan/0";
    //cout << filename << endl;
    size_t chan_size;
    decompress_zstd_file(filename, chan, chan_size);
    
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    //cout << "Elapsed time " << duration << endl;
    //cout << "The file name is " << filename << endl;
    
    // Print the converted complex values
    //cout << "vis_data_size " << vis_size << ",*," << uvw_size << ",*," << weight_size << ",*," << chan_size << endl;

}
