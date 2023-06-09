import zarr
import copy
import xarray as xr
DIMENSION_KEY = "_ARRAY_DIMENSIONS"
import numcodecs

def _open_no_dask_zarr(zarr_name, slice_dict={}, var_select=None):
    """
    Alternative to xarray open_zarr where the arrays are not Dask Arrays.

    slice_dict: A dictionary of slice objects for which values to read form a dimension.
                For example silce_dict={'time':slice(0,10)} would select the first 10 elements in the time dimension.
                If a dim is not specified all values are returned.
    return:
        xarray.Dataset()
    """

    zarr_group = zarr.open_group(store=zarr_name, mode="r")
    group_attrs = _get_attrs(zarr_group)

    slice_dict_complete = copy.deepcopy(slice_dict)
    coords = {}
    xds = xr.Dataset()

    for var_name, var in zarr_group.arrays():
        var_attrs = _get_attrs(var)

        if (var_select is None) or (var_name in var_select) or (var_name.islower()): #islower() checks if it is a coordinate, not ideal.
            for dim in var_attrs[DIMENSION_KEY]:
                if dim not in slice_dict_complete:
                    slice_dict_complete[dim] = slice(None)  # No slicing.

            if (var_attrs[DIMENSION_KEY][0] == var_name) and (
                len(var_attrs[DIMENSION_KEY]) == 1
            ):
                coords[var_name] = var[
                    slice_dict_complete[var_attrs[DIMENSION_KEY][0]]
                ]  # Dimension coordinates.
            else:
                # Construct slicing
                slicing_list = []
                for dim in var_attrs[DIMENSION_KEY]:
                    slicing_list.append(slice_dict_complete[dim])
                slicing_tuple = tuple(slicing_list)
                xds[var_name] = xr.DataArray(
                    var[slicing_tuple], dims=var_attrs[DIMENSION_KEY]
                )

    xds = xds.assign_coords(coords)

    xds.attrs = group_attrs
    return xds

def _get_attrs(zarr_obj):
    """Get attributes of zarr obj (groups or arrays)

    Args:
        zarr_obj (zarr): a zarr_group object

    Returns:
        dict: a group of zarr attibutes
    """
    return {
        k: v for k, v in zarr_obj.attrs.asdict().items() if not k.startswith("_NC")
    }




def decompressZstdFile(input_file):
    # Create a Zstd codec instance
    codec = numcodecs.Zstd()

    # Read the compressed data from the input file
    with open(input_file, 'rb') as f:
        compressed_data = f.read()

    # Decompress the data
    decompressed_data = codec.decode(compressed_data)

    return compressed_data,decompressed_data
