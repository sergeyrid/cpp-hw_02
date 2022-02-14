#include <array>
#include <climits>
#include <fstream>
#include <list>
#include <memory>
#include <queue>
#include <string>
#include <vector>


namespace huffman_algo {

class HuffmanArchiver final {
   class TreeNode;
   class HuffTree;

public:
    HuffmanArchiver(const std::string &in_filename, const std::string &out_filename);
    HuffmanArchiver(const HuffmanArchiver &other) = delete;
    ~HuffmanArchiver() = default;

    uint32_t get_in_file_size() const noexcept;
    uint32_t get_out_file_size() const noexcept;
    uint32_t get_extra_data_size() const noexcept;

    void zip();
    void unzip();

private:
    std::ifstream _in;
    std::ofstream _out;
    uint32_t _in_file_size;
    uint32_t _out_file_size;
    uint32_t _extra_data_size;

    std::array<uint32_t, UCHAR_MAX + 1> build_vocabulary();
    std::array<uint32_t, UCHAR_MAX + 1> extract_vocabulary();
    void decode(HuffTree &tree);
    void encode(HuffTree &tree);
    void fill_buffer(std::queue<bool> &buffer);
    void extract_buffer(std::queue<bool> &buffer);

    class TestHuffmanArchiver;
};


class HuffmanArchiver::TreeNode final {
public:
    explicit TreeNode(uint32_t frequency = 0, bool is_leaf = false, unsigned char value = 0,
                      std::unique_ptr<TreeNode> left_child = nullptr,
                      std::unique_ptr<TreeNode> right_child = nullptr) noexcept;
    TreeNode(const TreeNode &other) = delete;
    ~TreeNode() = default;

    uint32_t get_frequency() const noexcept;
    bool is_leaf() const noexcept;
    unsigned char get_value() const noexcept;
    const std::unique_ptr<TreeNode> &get_left_child() const noexcept;
    const std::unique_ptr<TreeNode> &get_right_child() const noexcept;

private:
    uint32_t _frequency;
    bool _is_leaf;
    unsigned char _value;
    const std::unique_ptr<TreeNode> _left_child;
    const std::unique_ptr<TreeNode> _right_child;

    class TestTreeNode;
};


class HuffmanArchiver::HuffTree final {
public:
    explicit HuffTree(const std::array<uint32_t, UCHAR_MAX + 1> &vocabulary);
    HuffTree(const HuffTree &other) = delete;
    ~HuffTree() = default;

    std::vector<bool> &get_code_by_char(unsigned char chr) noexcept;
    bool try_extract_code(std::queue<bool> &buffer, unsigned char &chr);

private:
    std::unique_ptr<TreeNode> _root;
    std::vector<bool> _chars_to_codes[UCHAR_MAX + 1];
    const TreeNode *_cur_node;

    static std::_List_iterator<std::unique_ptr<TreeNode>> find_min(std::list<std::unique_ptr<TreeNode>> &nodes);
    static std::unique_ptr<TreeNode> build_tree(const std::array<uint32_t, UCHAR_MAX + 1> &vocabulary);
    void get_codes();
    void get_next_code(std::vector<bool> &code);

    class TestHuffTree;
};

}
