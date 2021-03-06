#ifndef lss_lss_utilities_hpp
#define lss_lss_utilities_hpp


#include <algorithm>
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <typeinfo>
#include <vector>


/* -- basic types for index manipulation ------------------------------------ */


/**
 * @brief Index pair, fundamental dereferencing type
 */
struct idx_t
{
  size_t i, j;
  idx_t(const size_t& _i=std::numeric_limits< size_t >::max(),
        const size_t& _j=std::numeric_limits< size_t >::max()) : i(_i), j(_j) {}

  bool operator<  (const idx_t& _other) const { return (i<_other.i? true : i>_other.i? false : (j<_other.j)); }
  bool operator>  (const idx_t& _other) const { return idx_t(_other)<*this; }
  bool operator== (const idx_t& _other) const { return i==_other.i && j==_other.j; }
  bool operator!= (const idx_t& _other) const { return i!=_other.i || j!=_other.j; }

  idx_t& invalidate() { return (*this = idx_t()); }
  bool is_valid_size()  const { return operator>(idx_t(0,0)) && operator<(idx_t()); }
  bool is_square_size() const { return i==j; }
  bool is_diagonal()    const { return is_square_size(); }
};


/**
 * @brief Indexing base conversion tool (functor)
 */
struct base_conversion_t
{
  base_conversion_t(const int& _diff) : diff (_diff) {}
  int& operator()(int& v) { return (v+=diff); }
  const int& diff;
};


/**
 * @brief Storage type conversion (POD to POD) (functor)
 */
template< typename Tin, typename Tout >
struct storage_conversion_t {
  Tout operator()(const Tin& in) { return static_cast< Tout >(in); }
};



/**
 * @brief Coordinate matrix entry
 */
typedef std::pair< idx_t, double > coord_t;


/**
 * @brief Coordinate matrix sorting tool, by row (functor)
 */
struct coord_ordering_by_row_t
{
  bool operator()(const coord_t& a, const coord_t& b) const {
    return (a.first) < (b.first);
  }
};


/**
 * @brief Coordinate matrix sorting tool, by column (functor)
 */
struct coord_ordering_by_column_t
{
  bool operator()(const coord_t& a, const coord_t& b) const {
    return (a.first.j<b.first.j? true  :
           (a.first.j>b.first.j? false : (a.first.i<b.first.i)));
  }
};


/**
 * @brief Coordinate matrix row compression tool (functor)
 */
struct coord_row_equal_to_t
{
  const size_t i;
  coord_row_equal_to_t(const size_t& _i) : i(_i) {}
  bool operator()(const coord_t& a) const { return i==a.first.i; }
};


/**
 * @brief Coordinate matrix column compression tool (functor)
 */
struct coord_column_equal_to_t
{
  const size_t j;
  coord_column_equal_to_t(const size_t& _j) : j(_j) {}
  bool operator()(const coord_t& a) const { return j==a.first.j; }
};


/* -- vector transformations by type ---------------------------------------- */


/**
 * @brief Type list for recursive application of vector transformations
 * (to be used without object)
 */

struct transform_list_t_end
{
  static void apply(std::vector< size_t >&, size_t) {}
};

template<
    typename Transf,
    typename TransfNested=transform_list_t_end >
struct transform_list_t
{
  static void apply(std::vector< size_t >& v, int e) {
    TransfNested::apply(v,e);
    Transf::apply(v,e);
  }
};


// Transformation operations for vectors
// (common operations for building row or column-oriented sparsity patterns)


struct vector_sort_unique
{
  static void apply(std::vector< size_t >& v, int) {
    std::sort(v.begin(),v.end());
    v.erase(std::unique(v.begin(),v.end()),v.end());
  }
};

struct vector_element_push_front
{
  static void apply(std::vector< size_t >& v, size_t e) { v.insert(v.begin(),e); }
};

struct vector_element_push_back
{
  static void apply(std::vector< size_t >& v, int e) { v.push_back(e); }
};

struct vector_element_remove
{
  struct equal_to
  {
    equal_to(const size_t& _elem) : elem(_elem) {}
    bool operator()(const size_t& _elem) const { return elem==_elem; }
    const size_t elem;
  };
  static void apply(std::vector< size_t >& v, int e) {
    v.erase(std::remove_if(v.begin(),v.end(),
      equal_to(static_cast< size_t >(e))),v.end());
  }
};

struct vector_add_value
{
  struct add_value
  {
    add_value(const int& _diff) : diff(_diff) {}
    size_t operator()(size_t &v) const {
      return static_cast< size_t >(static_cast< int >(v)+diff);
    }
    const int diff;
  };
  static void apply(std::vector< size_t >& v, int e) {
    std::for_each(v.begin(),v.end(),add_value(e));
  }
};


/**
 * @brief Vector transformation: sorted indices vector
 * (useful for CSR matrix linear solvers)
 */
typedef transform_list_t< vector_sort_unique,
        transform_list_t< vector_element_push_back > >
  vector_sorted_t;

/**
 * @brief Vector transformation: sorted indices vector, with a particular entry
 * placed first (useful for specific CSR matrix linear solvers)
 */
typedef transform_list_t< vector_element_push_front,
        transform_list_t< vector_sort_unique,
        transform_list_t< vector_element_remove > > >
  vector_sorted_diagonal_first_t;


/* -- Matrix Market I/O (or, say, just I) ----------------------------------- */

namespace MatrixMarket
{


/**
 * @brief read_dense: read MatrixMarket file to dense structure
 * @param fname: input filename
 * @param roworiented: if result should be row (most common) or column oriented
 * @param size: output matrix/array size
 * @param a: output dense matrix/array
 * @return if reading is successful
 */
bool read_dense(
    const std::string& fname,
    const bool &roworiented,
    idx_t& size,
    std::vector< std::vector< double > >& a );


/**
 * @brief read_sparse: read MatrixMarket file to sparse structure
 * @param fname: input filename
 * @param roworiented: if result should be row (most common) or column oriented
 * @param base: sparse structure index base
 * @param size: output matrix/array size
 * @param a: output sparse matrix/array
 * @param ia: output i-coordinate indices
 * @param ja: output j-coordinate indices
 * @return if reading is successful
 */
bool read_sparse(
    const std::string& fname,
    const bool& roworiented,
    const int& base,
    idx_t& size,
    std::vector< double >& a,
    std::vector< int >& ia,
    std::vector< int >& ja );


}


/* -- Harwell-Boeing I/O (or, say again, just I) ---------------------------- */

namespace HarwellBoeing
{


/**
 * @brief read_dense: read Harwell-Boeing file format to dense structure
 * @param fname: input filename
 * @param roworiented: if result should be row (most common) or column oriented
 * @param size: output matrix/array size
 * @param a: output dense matrix/array
 * @return if reading is successful
 */
bool read_dense(
    const std::string& fname,
    const bool &roworiented,
    idx_t& size,
    std::vector< std::vector< double > >& a );


/**
 * @brief read_sparse: read Harwell-Boeing file format to sparse structure
 * @param fname: input filename
 * @param roworiented: if result should be row (most common) or column oriented
 * @param base: sparse structure index base
 * @param size: output matrix/array size
 * @param a: output sparse matrix/array
 * @param ia: output i-coordinate indices
 * @param ja: output j-coordinate indices
 * @return if reading is successful
 */
bool read_sparse(
    const std::string& fname,
    const bool& roworiented,
    const int& base,
    idx_t& size,
    std::vector< double >& a,
    std::vector< int >& ia,
    std::vector< int >& ja );


}


/* -- CSR I/O (just I) ------------------------------------------------------ */

namespace CSR
{


/**
 * @brief read_dense: read CSR file format (a hack on MM) to dense structure
 * @param fname: input filename
 * @param roworiented: if result should be row (most common) or column oriented
 * @param size: output matrix/array size
 * @param a: output dense matrix/array
 * @return if reading is successful
 */
bool read_dense(
    const std::string& fname,
    const bool &roworiented,
    idx_t& size,
    std::vector< std::vector< double > >& a );


/**
 * @brief read_sparse: read CSR file format (a hack on MM) to sparse structure
 * @param fname: input filename
 * @param roworiented: if result should be row (most common) or column oriented
 * @param base: sparse structure index base
 * @param size: output matrix/array size
 * @param a: output sparse matrix/array
 * @param ia: output i-coordinate indices
 * @param ja: output j-coordinate indices
 * @return if reading is successful
 */
bool read_sparse(
    const std::string& fname,
    const bool& roworiented,
    const int& base,
    idx_t& size,
    std::vector< double >& a,
    std::vector< int >& ia,
    std::vector< int >& ja );


}


/* -- Generic I/O (interfacing the above) ----------------------------------- */


/**
 * @brief read_dense: interface reading of files in different formats to dense
 * data structure, templasized with the container storage type
 * @param fname: input filename
 * @param roworiented: if result should be row (most common) or column oriented
 * @param size: output matrix/array size
 * @param a: output dense matrix/array
 * @return if reading is successful
 */
template< typename T >
bool read_dense(
    const std::string& fname,
    const bool &roworiented,
    idx_t& size,
    std::vector< std::vector< T > >& a )
{
  // if storage is of different type than double, it miraculously converts
  const bool storage_conversion_needed(typeid(T)!=typeid(double));
  std::vector< std::vector< double > > another_a;
  std::vector< std::vector< double > >& storage(
        storage_conversion_needed? another_a
                                 : (std::vector< std::vector< double > >&) a);

  // read file contents
  if      (fname.substr(fname.find_last_of("."))==".mtx") { MatrixMarket ::read_dense(fname,roworiented,size,storage); }
  else if (fname.substr(fname.find_last_of("."))==".rua") { HarwellBoeing::read_dense(fname,roworiented,size,storage); }
  else if (fname.substr(fname.find_last_of("."))==".csr") { CSR          ::read_dense(fname,roworiented,size,storage); }
  else {
    throw std::runtime_error("file format not detected (\""+fname+"\").");
  }

  // perform storage conversion if necessary, and return
  if (storage_conversion_needed) {
    a.assign(roworiented? size.i:size.j,std::vector< T >(
             roworiented? size.j:size.i,T() ));
    for (size_t i=0; i<another_a.size(); ++i)
      transform( another_a[i].begin(),another_a[i].end(),a[i].begin(),
                 storage_conversion_t< double, T >() );
  }
  return true;
}


/**
 * @brief read_sparse: interface reading of files in different formats to sparse
 * data structure, templasized with the container storage type
 * @param fname: input filename
 * @param roworiented: if result should be row (most common) or column oriented
 * @param base: sparse structure index base
 * @param size: output matrix/array size
 * @param a: output sparse matrix/array
 * @param ia: output i-coordinate indices
 * @param ja: output j-coordinate indices
 * @return if reading is successful
 */
template< typename T >
bool read_sparse(
    const std::string& fname,
    const bool& roworiented,
    const int& base,
    idx_t& size,
    std::vector< T >& a,
    std::vector< int >& ia,
    std::vector< int >& ja )
{
  // if storage is of different type than double, it miraculously converts
  const bool storage_conversion_needed(typeid(T)!=typeid(double));
  std::vector< double > another_a;
  std::vector< double >& storage(
        storage_conversion_needed? another_a : (std::vector< double >&) a);

  // read file contents
  if      (fname.substr(fname.find_last_of("."))==".mtx") { MatrixMarket ::read_sparse(fname,roworiented,base,size,storage,ia,ja); }
  else if (fname.substr(fname.find_last_of("."))==".rua") { HarwellBoeing::read_sparse(fname,roworiented,base,size,storage,ia,ja); }
  else if (fname.substr(fname.find_last_of("."))==".csr") { CSR          ::read_sparse(fname,roworiented,base,size,storage,ia,ja); }
  else {
    throw std::runtime_error("file format not detected (\""+fname+"\").");
  }

  // perform storage conversion if necessary, and return
  if (storage_conversion_needed) {
    a.assign(another_a.size(),T());
    transform( another_a.begin(),another_a.end(),a.begin(),
               storage_conversion_t< double, T >() );
  }
  return true;
}


#endif
