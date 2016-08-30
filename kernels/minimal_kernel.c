__kernel void min_kern(
        global char* in,
        global char* map,
        global int3 map_dim)
{
    int a = 10;
    size_t id = get_global_id(0);
    printf("%c%i\n", in[id], id);
}