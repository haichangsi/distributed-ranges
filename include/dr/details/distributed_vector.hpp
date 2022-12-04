namespace lib {

template <typename Container> struct xreference;
template <typename Container> struct const_xreference;

template <typename Container> struct const_xpointer {
  using T = typename Container::element_type;
  using const_reference = const_xreference<Container>;

  // Required for random access iterator
  using value_type = typename Container::value_type;
  using size_type = typename Container::size_type;
  using difference_type = typename Container::difference_type;

  // Pointer arithmetic
  bool operator==(const const_xpointer &other) const noexcept {
    return index_ == other.index_ && container_ == other.container_;
  }
  bool operator<=>(const const_xpointer &other) const noexcept;
  const_xpointer &operator++() {
    index_++;
    return *this;
  }
  const_xpointer operator++(int);
  const_xpointer &operator--();
  const_xpointer operator--(int);
  difference_type operator-(const const_xpointer &other) const noexcept;
  const_xpointer &operator-=(difference_type n);
  const_xpointer &operator+=(difference_type n);
  const_xpointer operator+(difference_type n) const noexcept {
    return const_xpointer{container_, index_ + n};
  }
  const_xpointer operator-(difference_type n) const noexcept;
  friend const_xpointer operator+(difference_type n,
                                  const const_xpointer &other) {
    return other + n;
  }

  T get() const { return container_->get(index_); }

  // dereference
  const_reference operator*() const { return const_reference{*this}; }
  const_reference operator[](difference_type n) const {
    return const_reference{*this + n};
  }

  const Container &object() { return *container_; }
  const Container *container_;
  std::size_t index_;
};

template <typename Container> struct xpointer {
  using T = typename Container::element_type;
  using reference = xreference<Container>;
  using const_reference = const_xreference<Container>;
  using const_pointer = const_xpointer<Container>;

  // Required for random access iterator
  using value_type = typename Container::value_type;
  using size_type = typename Container::size_type;
  using difference_type = typename Container::difference_type;

  // Pointer arithmetic
  bool operator==(const xpointer &other) const noexcept {
    return index_ == other.index_ && container_ == other.container_;
  }
  bool operator<=>(const xpointer &other) const noexcept;
  xpointer &operator++() {
    index_++;
    return *this;
  }
  xpointer operator++(int);
  xpointer &operator--();
  xpointer operator--(int);
  difference_type operator-(const xpointer &other) const noexcept;
  xpointer &operator-=(difference_type n) const noexcept;
  xpointer &operator+=(difference_type n) const noexcept;
  xpointer operator+(difference_type n) const noexcept {
    return xpointer{container_, index_ + n};
  }
  xpointer operator-(difference_type n) const noexcept {
    return xpointer{container_, index_ - n};
  }

  friend xpointer operator+(difference_type n, const xpointer &other) {
    return other + n;
  }

  // dereference
  reference operator*() const { return reference{*this}; }
  reference operator[](difference_type n) const { return reference{*this + n}; }

  // get/set
  T get() const { return container_->get(index_); }
  void put(T val) const { container_->put(index_, val); }

  operator const_pointer() const { return const_pointer{container_, index_}; }

  Container &object() { return *container_; }

  Container *container_;
  std::size_t index_;
};

template <typename Container> struct const_xreference {
  using T = typename Container::element_type;
  using const_pointer = const_xpointer<Container>;

  operator T() const { return pointer_.get(); }
  const_pointer operator&() const { return pointer_; }

  const_pointer pointer_;
};

template <typename Container> struct xreference {
  using T = typename Container::element_type;
  using const_reference = const_xreference<Container>;
  using pointer = xpointer<Container>;
  using const_pointer = const_xpointer<Container>;

  operator T() const { return pointer_.get(); }
  xreference operator=(xreference &r) {
    *this = T(r);
    return *this;
  }
  xreference operator=(const T &value) const {
    pointer_.put(value);
    return *this;
  }
  pointer operator&() const { return pointer_; }
  operator const_reference() {
    return const_reference{const_pointer(pointer_)};
  }

  pointer pointer_;
};

template <typename T, typename Alloc = std::allocator<T>,
          typename D = block_cyclic>
class distributed_vector {
public:
  using element_type = T;

  /// Type of elements stored in distributed_vector
  using value_type = T;

  /// Type used for storing sizes
  using size_type = std::size_t;
  /// Type used for storing differences
  using difference_type = std::ptrdiff_t;

  /// Type of decomposition
  using decomposition = D;

#ifdef DR_SPEC

  /// Pointer
  using pointer = implementation_defined;
  /// Pointer to immutable
  using const_pointer = implementation_defined;

#else

  // Could be GPU memory remote cpu memory. Pointer memory references
  // implemented by a constructor object or template parameter and
  // specialization

  /// Pointer type
  using pointer = xpointer<distributed_vector>;
  /// Const pointer type
  using const_pointer = const_xpointer<distributed_vector>;

#endif

  /// Reference to element
  using reference = std::iter_reference_t<pointer>;
  // using reference = std::iter_reference_t<xpointer<distributed_vector>>;
  /// Reference to immutable element
  using const_reference = std::iter_reference_t<const_pointer>;

  /// Iterator type
  using iterator = pointer;

  /// Read-only iterator
  using const_iterator = const_pointer;

  /// Stencil specification
  using stencil_type = stencil<1>;

  /// Allocator
  using allocator_type = Alloc;

  /// Construct a distributed vector with `count` elements.
  distributed_vector(D decomp, size_type count)
      : decomp_(decomp), size_(count), comm_(decomp.comm()),
        local_(local_storage_size()) {
    init();
  }

  /// Construct a distributed vector with a halo and `count` elements.
  distributed_vector(stencil_type s, size_type count)
      : stencil_(s), size_(count), comm_(decomp_.comm()),
        local_(local_storage_size()), halo_(comm_, local_, stencil_) {
    init();
  }

  /// Construct a distributed vector with a halo and `count` elements.
  distributed_vector(stencil_type s, Alloc alloc, size_type count)
      : stencil_(s), size_(count), comm_(decomp_.comm()),
        local_(local_storage_size(), alloc), halo_(comm_, local_, stencil_) {
    init();
  }

  /// Construct a distributed vector with a halo and `count` elements.
  distributed_vector(std::size_t radius, bool periodic, size_type count)
      : stencil_(radius), size_(count), comm_(decomp_.comm()),
        local_(local_storage_size()), halo_(comm_, local_, stencil_) {
    init();
  }

  /// Construct a distributed vector with `count` elements.
  distributed_vector(size_type count)
      : size_(count), comm_(decomp_.comm()), local_(local_storage_size()) {
    init();
  }

  /// Construct a distributed vector with `count` elements.
  distributed_vector(Alloc alloc, size_type count)
      : size_(count), comm_(decomp_.comm()),
        local_(local_storage_size(), alloc) {
    init();
  }

  /// Construct a distributed vector with `count` elements equal to `value`.
  distributed_vector(size_type count, T value, D decomp = D{}) {
    assert(false);
  }

  ~distributed_vector() {
    fence();
    win_.free();
  }

  /// copy a span to a distributed vector
  void scatter(const std::span<T> src, int root) {
    assert(comm_.rank() != root || comm_.size() * local_.size() == src.size());
    comm_.scatter(src.data(), local_.data(), local_.size() * sizeof(T), root);
  }

  /// copy a distributed vector to a span
  void gather(const std::span<T> dst, int root) {
    assert(comm_.rank() != root || comm_.size() * local_.size() == dst.size());
    comm_.gather(local_.data(), dst.data(), local_.size() * sizeof(T), root);
  }

  T get(std::size_t index) const {
    auto [rank, offset] = rank_offset(index);
    auto val = win_.get<T>(rank, offset);
    drlog.debug("get {} =  [{}]\n", val, index);
    return val;
  }

  void put(std::size_t index, const T &val) {
    drlog.debug("put [{}] = {}\n", index, val);
    auto [rank, offset] = rank_offset(index);
    win_.put(val, rank, offset);
  }

  /// Index into a distributed vector
  reference operator[](const size_t index) { return *iterator{this, index}; }

  /// Index into a distributed vector
  const_reference operator[](const size_t index) const {
    return *const_iterator{this, index};
  }

  iterator begin() { return iterator{this, 0}; }
  const_iterator begin() const { return const_iterator{this, 0}; }
  iterator end() { return iterator{this, size_}; };
  const_iterator end() const { return const_iterator{this, size_}; };

  void fence() { win_.fence(); }

  void flush(int rank) { win_.flush(rank); }

  const std::span<T> local() { return local_; }
  const std::span<const T> local() const { return local_; }

  const std::span<lib::remote_vector<T>> &segments() const;

  size_type size() const { return size_; }

  span_halo<T> &halo() { return halo_; }

  communicator &comm() { return comm_; }

  bool conforms(const distributed_vector &other) const noexcept {
    return decomp_ == other.decomp_ && size_ == other.size_;
  }

  bool congruent(const iterator &first, const iterator &last) const noexcept {
    return first == begin() && last == end();
  }

  bool congruent(const iterator &first) const noexcept {
    return first == begin();
  }

private:
  auto rank_offset(std::size_t index) const {
    auto radius = stencil_.radius()[0];
    std::size_t slice_size = local().size() - radius.prev - radius.next;
    std::size_t rank, offset;
    if (index < radius.prev) {
      rank = 0;
    } else if (index >= size() - radius.next) {
      rank = comm_.size() - 1;
    } else {
      rank = (index - radius.prev) / slice_size;
    }
    offset = index - rank * slice_size;

    return std::pair(rank, offset);
  }

  void init() {
#ifdef OMPI_MAJOR_VERSION
    // openmpi cannot create a window when the size is 1
    // Not sure if it is specific to the version that comes with ubuntu 22
    assert(comm_.size() > 1);
#endif
    win_.create(comm_, local_.data(), local_.size() * sizeof(T));
    fence();
  }

  auto local_storage_size() const {
    auto radius = stencil_.radius()[0];
    return partition_up(size_ - radius.prev - radius.next, comm_.size()) +
           radius.prev + radius.next;
  }

  stencil_type stencil_;
  D decomp_;
  size_type size_;
  communicator comm_;
  std::vector<T, Alloc> local_;
  communicator::win win_;
  span_halo<T> halo_;
};

} // namespace lib
