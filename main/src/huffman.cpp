#include <array>
#include <climits>
#include <fstream>
#include <list>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>
#include "huffman.h"

using namespace huffman_algo;

huffman_algo::HuffmanArchiver::TreeNode::TreeNode(uint32_t frequency, bool is_leaf, unsigned char value,
                   std::unique_ptr<TreeNode> left_child, std::unique_ptr<TreeNode> right_child) noexcept:
        _frequency(frequency), _is_leaf(is_leaf), _value(value),
        _left_child(std::move(left_child)), _right_child(std::move(right_child)) { }

uint32_t HuffmanArchiver::TreeNode::get_frequency() const noexcept {
    return _frequency;
}

bool HuffmanArchiver::TreeNode::is_leaf() const noexcept {
    return _is_leaf;
}

unsigned char HuffmanArchiver::TreeNode::get_value() const noexcept {
    return _value;
}

const std::unique_ptr<HuffmanArchiver::TreeNode> &HuffmanArchiver::TreeNode::get_left_child() const noexcept {
    return _left_child;
}

const std::unique_ptr<HuffmanArchiver::TreeNode> &HuffmanArchiver::TreeNode::get_right_child() const noexcept {
    return _right_child;
}

HuffmanArchiver::HuffTree::HuffTree(const std::array<uint32_t, UCHAR_MAX + 1> &vocabulary) {
    _root = build_tree(vocabulary);
    get_codes();
    _cur_node = _root.get();
}

std::vector<bool> &HuffmanArchiver::HuffTree::get_code_by_char(unsigned char chr) noexcept {
    return _chars_to_codes[chr];
}

bool HuffmanArchiver::HuffTree::try_extract_code(std::queue<bool> &buffer, unsigned char &chr) {
    if (_cur_node && _cur_node == _root.get() && _cur_node->is_leaf() && !buffer.empty()) {
        buffer.pop();
        chr = _cur_node->get_value();
        return true;
    }
    while (_cur_node && !_cur_node->is_leaf() && !buffer.empty()) {
        bool bit = buffer.front();
        if (bit) {
            _cur_node = _cur_node->get_left_child().get();
        } else {
            _cur_node = _cur_node->get_right_child().get();
        }
        buffer.pop();
    }
    if (!_cur_node) {
        throw std::logic_error("Attempt to extract a code from invalid data.");
    } else if (_cur_node->is_leaf()) {
        chr = _cur_node->get_value();
        _cur_node = _root.get();
        return true;
    }
    return false;
}

std::_List_iterator<std::unique_ptr<HuffmanArchiver::TreeNode>>
        HuffmanArchiver::HuffTree::find_min(std::list<std::unique_ptr<TreeNode>> &nodes) {
    auto min = nodes.begin();
    for (auto it = nodes.begin(); it != nodes.end(); ++it) {
        if ((*it)->get_frequency() < (*min)->get_frequency()) {
            min = it;
        }
    }
    return min;
}

std::unique_ptr<HuffmanArchiver::TreeNode>
        HuffmanArchiver::HuffTree::build_tree(const std::array<uint32_t, UCHAR_MAX + 1> &vocabulary) {
    std::size_t vocabulary_size = 0;
    std::list<std::unique_ptr<TreeNode>> nodes;
    for (std::size_t i = 0; i <= UCHAR_MAX; ++i) {
        if (vocabulary.at(i)) {
            ++vocabulary_size;
            nodes.push_back(std::make_unique<TreeNode>(vocabulary.at(i), true, i));
        }
    }
    while (nodes.size() >= 2) {
        auto first_min = find_min(nodes);
        std::unique_ptr<TreeNode> left_child = std::move(*first_min);
        nodes.erase(first_min);
        auto second_min = find_min(nodes);
        std::unique_ptr<TreeNode> right_child = std::move(*second_min);
        nodes.erase(second_min);
        std::unique_ptr<TreeNode> node =
                std::make_unique<TreeNode>(left_child->get_frequency() + right_child->get_frequency(),
                                           false, 0, std::move(left_child), std::move(right_child));
        nodes.push_back(std::move(node));
    }
    return nodes.empty() ? nullptr : std::move(nodes.front());
}

void HuffmanArchiver::HuffTree::get_codes() {
    _cur_node = _root.get();
    std::vector<bool> code;
    get_next_code(code);
}

void HuffmanArchiver::HuffTree::get_next_code(std::vector<bool> &code) {
    if (!_cur_node) {
        return;
    }
    if (code.empty() && _cur_node->is_leaf()) {
        code.push_back(true);
    }
    if (_cur_node->is_leaf()) {
        _chars_to_codes[_cur_node->get_value()] = code;
        return;
    }
    code.push_back(true);
    const TreeNode *tmp_node = _cur_node;
    _cur_node = tmp_node->get_left_child().get();
    get_next_code(code);
    code.pop_back();
    code.push_back(false);
    _cur_node = tmp_node->get_right_child().get();
    get_next_code(code);
    code.pop_back();
}

HuffmanArchiver::HuffmanArchiver(const std::string &in_filename, const std::string &out_filename):
        _in_file_size(0), _out_file_size(0), _extra_data_size(0) {
    _in = std::ifstream(in_filename, std::ios_base::binary);
    if (!_in) {
        throw std::invalid_argument("Couldn't open file \"" + in_filename + "\".");
    }
    _out = std::ofstream(out_filename, std::ios_base::binary);
    if (!_out) {
        throw std::invalid_argument("Couldn't open file \"" + out_filename + "\".");
    }
    _out.exceptions(std::ios_base::badbit | std::ios_base::failbit);
}

std::uint32_t HuffmanArchiver::get_in_file_size() const noexcept {
    return _in_file_size;
}

std::uint32_t HuffmanArchiver::get_out_file_size() const noexcept {
    return _out_file_size;
}

std::uint32_t HuffmanArchiver::get_extra_data_size() const noexcept {
    return _extra_data_size;
}

void HuffmanArchiver::zip() {
    _in.exceptions(std::ios_base::goodbit);
    std::array<uint32_t, UCHAR_MAX + 1> vocabulary = build_vocabulary();
    HuffTree tree(vocabulary);
    _in.clear();
    _in_file_size = _in.tellg();
    _out.write((char *)&_in_file_size, sizeof(_in_file_size));
    uint32_t vocabulary_size = 0;
    std::vector<std::pair<unsigned char, uint32_t>> records;
    for (std::size_t i = 0; i <= UCHAR_MAX; ++i) {
        if (vocabulary.at(i)) {
            ++vocabulary_size;
            records.emplace_back(i, vocabulary.at(i));
        }
    }
    _out.write((char *)&vocabulary_size, sizeof(vocabulary_size));
    for (auto &record: records) {
        _out.write((char *)&record.first, sizeof(record.first));
        _out.write((char *)&record.second, sizeof(record.second));
    }
    _extra_data_size = _out.tellp();
    encode(tree);
    _out_file_size = uint32_t(_out.tellp()) - _extra_data_size;
    _out.flush();
}

void HuffmanArchiver::unzip() {
    _in.exceptions(std::ios_base::badbit | std::ios_base::failbit);
    _in.read((char *)&_out_file_size, sizeof(_out_file_size));
    std::array<uint32_t, UCHAR_MAX + 1> vocabulary = extract_vocabulary();
    _extra_data_size = _in.tellg();
    HuffTree tree(vocabulary);
    decode(tree);
    _in_file_size = uint32_t(_in.tellg()) - _extra_data_size;
    _out.flush();
}

std::array<uint32_t, UCHAR_MAX + 1> HuffmanArchiver::build_vocabulary() {
    _in.exceptions(std::ios_base::goodbit);
    std::array<uint32_t, UCHAR_MAX + 1> vocabulary{};
    unsigned char chr;
    while (_in.read((char *)&chr, sizeof(chr))) {
        ++vocabulary.at(chr);
    }
    return vocabulary;
}

std::array<uint32_t, UCHAR_MAX + 1> HuffmanArchiver::extract_vocabulary() {
    _in.exceptions(std::ios_base::badbit | std::ios_base::failbit);
    std::array<uint32_t, UCHAR_MAX + 1> vocabulary{};
    uint32_t vocabulary_size;
    _in.read((char *)&vocabulary_size, sizeof(vocabulary_size));
    for (std::size_t i = 0; i < vocabulary_size; ++i) {
        unsigned char chr;
        uint32_t frequency;
        _in.read((char *)&chr, sizeof(chr));
        _in.read((char *)&frequency, sizeof(frequency));
        vocabulary.at(chr) = frequency;
    }
    return vocabulary;
}

void HuffmanArchiver::fill_buffer(std::queue<bool> &buffer) {
    unsigned char chr;
    _in.read((char *)&chr, sizeof(chr));
    for (std::size_t i = 0; i < CHAR_BIT; ++i) {
        buffer.push(chr & (1 << i));
    }
}

void HuffmanArchiver::decode(HuffTree &tree) {
    _in.exceptions(std::ios_base::badbit | std::ios_base::failbit);
    std::queue<bool> buffer;
    unsigned char chr;
    std::size_t i = 0;
    while (i < _out_file_size) {
        if (buffer.empty()) {
            fill_buffer(buffer);
        }
        if (tree.try_extract_code(buffer, chr)) {
            _out.write((char *)&chr, sizeof(chr));
            ++i;
        }
    }
}

void HuffmanArchiver::extract_buffer(std::queue<bool> &buffer) {
    while (buffer.size() >= CHAR_BIT) {
        unsigned char chr = 0;
        for (std::size_t i = 0; i < CHAR_BIT; ++i) {
            chr |= buffer.front() << i;
            buffer.pop();
        }
        _out.write((char *)&chr, sizeof(chr));
    }
}

void HuffmanArchiver::encode(HuffTree &tree) {
    _in.clear();
    _in.seekg(0);
    _in.exceptions(std::ios_base::goodbit);
    std::queue<bool> buffer;
    unsigned char chr;
    while (_in.read((char *)&chr, sizeof(chr))) {
        extract_buffer(buffer);
        for (auto bit: tree.get_code_by_char(chr)) {
            buffer.push(bit);
        }
    }
    while (buffer.size() % CHAR_BIT != 0) {
        buffer.push(false);
    }
    extract_buffer(buffer);
}
