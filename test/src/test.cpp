#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#include <array>
#include <climits>
#include <deque>
#include <fstream>
#include <list>
#include <memory>
#include <queue>
#include <stdexcept>
#include <string>
#include <vector>
#include "huffman.h"

using namespace huffman_algo;

static std::string path(const std::string &filename) {
    return DATA_DIR + filename;
}

class huffman_algo::HuffmanArchiver::TreeNode::TestTreeNode {
    TEST_CASE_CLASS("testing TreeNode") {
        SUBCASE("constructor doesn't throw") {
            CHECK_NOTHROW(TreeNode node);
            CHECK_NOTHROW(TreeNode node(2));
            CHECK_NOTHROW(TreeNode node(2, true));
            CHECK_NOTHROW(TreeNode node(2, true, 'a'));
            CHECK_NOTHROW(TreeNode node(2, true, 'a'));

            std::unique_ptr<TreeNode> left_child = nullptr;
            std::unique_ptr<TreeNode> right_child = std::make_unique<TreeNode>();

            CHECK_NOTHROW(TreeNode node(2, true, 'a', std::move(left_child)));
            CHECK_NOTHROW(TreeNode node(2, true, 'a', std::move(left_child), std::move(right_child)));
        }

        SUBCASE("constructor") {
            std::unique_ptr<TreeNode> left_child = nullptr;
            std::unique_ptr<TreeNode> right_child =
                    std::make_unique<TreeNode>(1, false, 'b',
                                               std::make_unique<TreeNode>(), nullptr);
            TreeNode node(2, true, 'a',
                          std::move(left_child), std::move(right_child));

            CHECK_EQ(node._frequency, 2);
            CHECK_EQ(node._is_leaf, true);
            CHECK_EQ(node._value, 'a');
            CHECK_EQ(node._left_child, nullptr);
            CHECK_NE(node._right_child, nullptr);
            CHECK_EQ(node._right_child->_frequency, 1);
            CHECK_EQ(node._right_child->_is_leaf, false);
            CHECK_EQ(node._right_child->_value, 'b');
            CHECK_NE(node._right_child->_left_child, nullptr);
            CHECK_EQ(node._right_child->_right_child, nullptr);
        }

        std::unique_ptr<TreeNode> left_child = std::make_unique<TreeNode>();
        std::unique_ptr<TreeNode> right_child = std::make_unique<TreeNode>();

        TreeNode default_node;
        const TreeNode const_node;
        TreeNode custom_node(2, true, 'a',
                             std::move(left_child),std::move(right_child));

        SUBCASE("get_frequency") {
            CHECK_EQ(default_node.get_frequency(), 0);
            CHECK_NOTHROW(const_node.get_frequency());
            CHECK_EQ(custom_node.get_frequency(), custom_node._frequency);
        }

        SUBCASE("is_leaf") {
            CHECK_EQ(default_node.is_leaf(), false);
            CHECK_NOTHROW(const_node.is_leaf());
            CHECK_EQ(custom_node.is_leaf(), custom_node._is_leaf);
        }

        SUBCASE("get_value") {
            CHECK_EQ(default_node.get_frequency(), 0);
            CHECK_NOTHROW(const_node.get_value());
            CHECK_EQ(custom_node.get_frequency(), custom_node._frequency);
        }

        SUBCASE("get_left_child") {
            CHECK_EQ(default_node.get_left_child(), nullptr);
            CHECK_EQ(const_node.get_left_child(), nullptr);
            CHECK_EQ(custom_node.get_left_child(), custom_node._left_child);
            CHECK_NE(custom_node.get_left_child(), custom_node._right_child);
        }

        SUBCASE("get_right_child") {
            CHECK_EQ(default_node.get_right_child(), nullptr);
            CHECK_EQ(const_node.get_right_child(), nullptr);
            CHECK_EQ(custom_node.get_right_child(), custom_node._right_child);
            CHECK_NE(custom_node.get_right_child(), custom_node._left_child);
        }
    }
};


class huffman_algo::HuffmanArchiver::HuffTree::TestHuffTree {
    TEST_CASE_CLASS("testing HuffTree") {
        std::array<uint32_t, UCHAR_MAX + 1> empty_vocabulary{};
        std::array<uint32_t, UCHAR_MAX + 1> normal_vocabulary{};
        normal_vocabulary['a'] = 100;
        normal_vocabulary['b'] = 200;
        normal_vocabulary['c'] = 300;
        std::array<uint32_t, UCHAR_MAX + 1> big_vocabulary{};
        std::size_t big_vocabulary_sum = 0;
        for (std::size_t i = 0; i <= UCHAR_MAX; ++i) {
            big_vocabulary[i] = 100 * (i + 1);
            big_vocabulary_sum += 100 * (i + 1);
        }
        REQUIRE_EQ(big_vocabulary.size(), UCHAR_MAX + 1);

        SUBCASE("constructor doesn't throw") {
            CHECK_NOTHROW(HuffTree tree(empty_vocabulary));
            CHECK_NOTHROW(HuffTree tree(normal_vocabulary));
            CHECK_NOTHROW(HuffTree tree(big_vocabulary));
        }

        SUBCASE("find_min") {
            std::list<std::unique_ptr<TreeNode>> empty_list;
            std::list<std::unique_ptr<TreeNode>> normal_list;
            normal_list.push_back(std::make_unique<TreeNode>(10));
            normal_list.push_back(std::make_unique<TreeNode>(5));
            normal_list.push_back(std::make_unique<TreeNode>(7));
            normal_list.push_back(std::make_unique<TreeNode>(2));
            normal_list.push_back(std::make_unique<TreeNode>(8));
            std::list<std::unique_ptr<TreeNode>> first_element_list;
            first_element_list.push_back(std::make_unique<TreeNode>(1));
            first_element_list.push_back(std::make_unique<TreeNode>(5));
            first_element_list.push_back(std::make_unique<TreeNode>(3));
            std::list<std::unique_ptr<TreeNode>> last_element_list;
            last_element_list.push_back(std::make_unique<TreeNode>(5));
            last_element_list.push_back(std::make_unique<TreeNode>(3));
            last_element_list.push_back(std::make_unique<TreeNode>(0));

            CHECK_EQ(find_min(empty_list), empty_list.end());
            CHECK_EQ(find_min(normal_list)->get()->get_frequency(), 2);
            CHECK_EQ(find_min(first_element_list)->get()->get_frequency(), 1);
            CHECK_EQ(find_min(last_element_list)->get()->get_frequency(), 0);
        }

        SUBCASE("build_tree") {
            std::unique_ptr<TreeNode> empty_root = HuffTree::build_tree(empty_vocabulary);
            std::unique_ptr<TreeNode> normal_root = HuffTree::build_tree(normal_vocabulary);
            std::unique_ptr<TreeNode> big_root = HuffTree::build_tree(big_vocabulary);

            CHECK_EQ(empty_root, nullptr);
            CHECK_EQ(normal_root->get_frequency(), 600);
            CHECK_NE(normal_root->get_left_child(), nullptr);
            CHECK_NE(normal_root->get_right_child(), nullptr);
            CHECK_EQ(big_root->get_frequency(), big_vocabulary_sum);
            CHECK_NE(big_root->get_left_child(), nullptr);
            CHECK_NE(big_root->get_right_child(), nullptr);
        }

        SUBCASE("constructor") {
            HuffTree empty_tree(empty_vocabulary);
            HuffTree normal_tree(normal_vocabulary);
            HuffTree big_tree(big_vocabulary);

            CHECK_EQ(empty_tree._root, nullptr);
            CHECK_NE(normal_tree._root, nullptr);
            CHECK_EQ(normal_tree._root->get_frequency(), 600);
            CHECK_NE(big_tree._root, nullptr);
            CHECK_EQ(big_tree._root->get_frequency(), big_vocabulary_sum);
            CHECK_EQ(empty_tree._cur_node, empty_tree._root.get());
            CHECK_EQ(normal_tree._cur_node, normal_tree._root.get());
            CHECK_EQ(big_tree._cur_node, big_tree._root.get());
        }

        SUBCASE("get_next_code") {
            HuffTree empty_tree(empty_vocabulary);
            empty_tree._cur_node = nullptr;
            HuffTree leaf_tree(empty_vocabulary);
            leaf_tree._cur_node = new TreeNode(20, true, 'a');
            std::vector<bool> code;

            CHECK_NOTHROW(empty_tree.get_next_code(code));
            REQUIRE(code.empty());
            CHECK_EQ(get_number_of_codes(empty_tree), 0);
            CHECK_NOTHROW(leaf_tree.get_next_code(code));
            CHECK_EQ(leaf_tree._chars_to_codes['a'].size(), 1);
            CHECK_EQ(get_number_of_codes(leaf_tree), 1);

            delete leaf_tree._cur_node;
        }

        SUBCASE("get_codes") {
            std::unique_ptr<TreeNode> empty_root = HuffTree::build_tree(empty_vocabulary);
            std::unique_ptr<TreeNode> normal_root = HuffTree::build_tree(normal_vocabulary);
            std::unique_ptr<TreeNode> big_root = HuffTree::build_tree(big_vocabulary);
            HuffTree empty_tree(empty_vocabulary);
            empty_tree._root = std::move(empty_root);
            HuffTree normal_tree(empty_vocabulary);
            normal_tree._root = std::move(normal_root);
            HuffTree big_tree(empty_vocabulary);
            big_tree._root = std::move(big_root);

            CHECK_NOTHROW(empty_tree.get_codes());
            CHECK_EQ(get_number_of_codes(empty_tree), 0);
            CHECK_NOTHROW(normal_tree.get_codes());
            CHECK_EQ(get_number_of_codes(normal_tree), 3);
            CHECK_EQ(normal_tree._chars_to_codes['a'].size(), 2);
            CHECK_EQ(normal_tree._chars_to_codes['b'].size(), 2);
            CHECK_EQ(normal_tree._chars_to_codes['c'].size(), 1);
            CHECK_NOTHROW(big_tree.get_codes());
            CHECK_EQ(get_number_of_codes(big_tree), 256);
        }

        HuffTree tree(empty_vocabulary);
        std::vector<bool> true_code(5, true);
        std::vector<bool> false_code(10, false);
        tree._chars_to_codes['a'] = true_code;
        tree._chars_to_codes['b'] = false_code;

        SUBCASE("get_code_by_char") {
            CHECK_EQ(tree.get_code_by_char('a'), true_code);
            CHECK_EQ(tree.get_code_by_char('b'), false_code);
            CHECK_EQ(tree.get_code_by_char('c').size(), 0);
        }

        SUBCASE("try_extract_code") {
            HuffTree empty_tree(empty_vocabulary);
            REQUIRE_EQ(empty_tree._cur_node, nullptr);
            std::array<uint32_t, UCHAR_MAX + 1> one_letter_vocabulary{};
            one_letter_vocabulary['a'] = 100;
            HuffTree one_letter_tree(one_letter_vocabulary);
            HuffTree normal_tree(normal_vocabulary);
            std::queue<bool> empty_buffer;
            std::queue<bool> false_buffer(std::deque(8, false));
            std::queue<bool> true_buffer(std::deque(8, true));
            std::queue<bool> big_buffer(std::deque(64, true));
            unsigned char chr;

            CHECK_THROWS_AS(empty_tree.try_extract_code(empty_buffer, chr), std::logic_error);
            CHECK(one_letter_tree.try_extract_code(false_buffer, chr));
            CHECK_EQ(false_buffer.size(), 7);
            CHECK_EQ(chr, 'a');
            CHECK(one_letter_tree.try_extract_code(true_buffer, chr));
            CHECK_EQ(true_buffer.size(), 7);
            CHECK_EQ(chr, 'a');
            CHECK(!normal_tree.try_extract_code(empty_buffer, chr));
            CHECK(normal_tree.try_extract_code(big_buffer, chr));
            CHECK_EQ(big_buffer.size(), 63);
            CHECK_EQ(chr, 'c');
        }
    }

    static std::size_t get_number_of_codes(const HuffTree &tree) {
        std::size_t code_size_sum = 0;
        for (auto &it: tree._chars_to_codes) {
            if (!it.empty()) {
                ++code_size_sum;
            }
        }
        return code_size_sum;
    }
};


class huffman_algo::HuffmanArchiver::TestHuffmanArchiver {
    TEST_CASE_CLASS("testing HuffmanArchiver") {
        std::string default_file;
        std::string no_file = path("no-file.txt");
        std::string empty_file = path("empty.txt");
        std::string normal_file = path("normal.txt");
        std::string one_letter_file = path("one letter.txt");
        std::string spaces_file = path("spaces.txt");
        std::string big_file = path("War and Peace.txt");
        std::string worst_file = path("worst.txt");
        std::string out_no_file = path("out no-file.txt");
        std::string zip_empty_file = path("zip empty.txt");
        std::string zip_normal_file = path("zip normal.txt");
        std::string zip_one_letter_file = path("zip one letter.txt");
        std::string zip_spaces_file = path("zip spaces.txt");
        std::string zip_big_file = path("zip War and Peace.txt");
        std::string zip_worst_file = path("zip worst.txt");
        std::string unzip_empty_file = path("unzip empty.txt");
        std::string unzip_normal_file = path("unzip normal.txt");
        std::string unzip_one_letter_file = path("unzip one letter.txt");
        std::string unzip_spaces_file = path("unzip spaces.txt");
        std::string unzip_big_file = path("unzip War and Peace.txt");
        std::string unzip_worst_file = path("unzip worst.txt");

        SUBCASE("constructor") {
            CHECK_THROWS_AS(HuffmanArchiver archiver(default_file, default_file), std::invalid_argument);
            CHECK_THROWS_AS(HuffmanArchiver archiver(no_file, out_no_file), std::invalid_argument);
            CHECK_NOTHROW(HuffmanArchiver archiver(empty_file, zip_empty_file));
            CHECK_NOTHROW(HuffmanArchiver archiver(normal_file, zip_normal_file));
            CHECK_NOTHROW(HuffmanArchiver archiver(big_file, zip_big_file));

            HuffmanArchiver archiver(normal_file, zip_normal_file);

            CHECK_EQ(archiver._out.exceptions(), std::ios_base::badbit | std::ios_base::failbit);
        }

        SUBCASE("zip mode") {
            HuffmanArchiver empty_archiver(empty_file, zip_empty_file);
            HuffmanArchiver normal_archiver(normal_file, zip_normal_file);
            HuffmanArchiver one_letter_archiver(one_letter_file, zip_one_letter_file);
            HuffmanArchiver spaces_archiver(spaces_file, zip_spaces_file);
            HuffmanArchiver big_archiver(big_file, zip_big_file);
            HuffmanArchiver worst_archiver(worst_file, zip_worst_file);

            SUBCASE("build_vocabulary") {
                std::array<uint32_t, UCHAR_MAX + 1> empty_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> normal_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> one_letter_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> spaces_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> big_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> worst_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> expected_empty_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> expected_normal_vocabulary{};
                expected_normal_vocabulary['a'] = 1;
                expected_normal_vocabulary['b'] = 1;
                expected_normal_vocabulary['c'] = 1;
                expected_normal_vocabulary['d'] = 1;
                expected_normal_vocabulary['e'] = 1;
                expected_normal_vocabulary['f'] = 1;
                std::array<uint32_t, UCHAR_MAX + 1> expected_one_letter_vocabulary{};
                expected_one_letter_vocabulary['a'] = 100;
                std::array<uint32_t, UCHAR_MAX + 1> expected_spaces_vocabulary{};
                expected_spaces_vocabulary[' '] = 10;
                expected_spaces_vocabulary['\n'] = 8;
                expected_spaces_vocabulary['a'] = 7;

                CHECK_NOTHROW(empty_vocabulary = empty_archiver.build_vocabulary());
                CHECK_NOTHROW(normal_vocabulary = normal_archiver.build_vocabulary());
                CHECK_NOTHROW(one_letter_vocabulary = one_letter_archiver.build_vocabulary());
                CHECK_NOTHROW(spaces_vocabulary = spaces_archiver.build_vocabulary());
                CHECK_NOTHROW(big_vocabulary = big_archiver.build_vocabulary());
                CHECK_NOTHROW(worst_vocabulary = worst_archiver.build_vocabulary());
                CHECK_EQ(empty_vocabulary, expected_empty_vocabulary);
                CHECK_EQ(normal_vocabulary, expected_normal_vocabulary);
                CHECK_EQ(one_letter_vocabulary, expected_one_letter_vocabulary);
                CHECK_EQ(spaces_vocabulary, expected_spaces_vocabulary);
            }

            SUBCASE("extract_buffer") {
                std::queue<bool> empty_buffer;
                std::queue<bool> false_buffer(std::deque(8, false));
                std::queue<bool> true_buffer(std::deque(8, true));
                std::queue<bool> a_buffer;
                for (std::size_t i = 0; i < 8; ++i) {
                    a_buffer.push('a' & (1 << i));
                }
                std::queue<bool> not_full_buffer(std::deque(7, false));
                std::queue<bool> bigger_buffer(std::deque(63, true));
                std::queue<bool> big_buffer(std::deque(2048, false));
                std::ifstream in(zip_empty_file, std::ios_base::binary);
                REQUIRE(in);

                CHECK_NOTHROW(empty_archiver.extract_buffer(empty_buffer));
                CHECK_NOTHROW(empty_archiver.extract_buffer(false_buffer));
                CHECK_NOTHROW(empty_archiver.extract_buffer(true_buffer));
                CHECK_NOTHROW(empty_archiver.extract_buffer(a_buffer));
                CHECK_NOTHROW(empty_archiver.extract_buffer(not_full_buffer));
                CHECK_NOTHROW(empty_archiver.extract_buffer(bigger_buffer));
                CHECK_NOTHROW(empty_archiver.extract_buffer(big_buffer));

                empty_archiver._out.flush();

                CHECK_EQ(empty_buffer.size(), 0);
                CHECK_EQ(false_buffer.size(), 0);
                CHECK_EQ(true_buffer.size(), 0);
                CHECK_EQ(a_buffer.size(), 0);
                CHECK_EQ(not_full_buffer.size(), 7);
                CHECK_EQ(bigger_buffer.size(), 7);
                CHECK_EQ(big_buffer.size(), 0);
                CHECK_EQ(in.get(), 0);
                CHECK_EQ(in.get(), 255);
                CHECK_EQ(in.get(), 'a');
                for (std::size_t i = 0; i < 7; ++i) {
                    CHECK_EQ(in.get(), 255);
                }
                for (std::size_t i = 0; i < 256; ++i) {
                    CHECK_EQ(in.get(), 0);
                }
            }

            SUBCASE("encode") {
                std::array<uint32_t, UCHAR_MAX + 1> empty_vocabulary = empty_archiver.build_vocabulary();
                std::array<uint32_t, UCHAR_MAX + 1> normal_vocabulary = normal_archiver.build_vocabulary();
                std::array<uint32_t, UCHAR_MAX + 1> one_letter_vocabulary = one_letter_archiver.build_vocabulary();
                std::array<uint32_t, UCHAR_MAX + 1> spaces_vocabulary = spaces_archiver.build_vocabulary();
                std::array<uint32_t, UCHAR_MAX + 1> big_vocabulary = big_archiver.build_vocabulary();
                std::array<uint32_t, UCHAR_MAX + 1> worst_vocabulary = worst_archiver.build_vocabulary();
                empty_archiver._in.clear();
                empty_archiver._in.seekg(0);
                normal_archiver._in.clear();
                normal_archiver._in.seekg(0);
                one_letter_archiver._in.clear();
                one_letter_archiver._in.seekg(0);
                spaces_archiver._in.clear();
                spaces_archiver._in.seekg(0);
                big_archiver._in.clear();
                big_archiver._in.seekg(0);
                worst_archiver._in.clear();
                worst_archiver._in.seekg(0);
                HuffTree empty_tree(empty_vocabulary);
                HuffTree normal_tree(normal_vocabulary);
                HuffTree one_letter_tree(one_letter_vocabulary);
                HuffTree spaces_tree(spaces_vocabulary);
                HuffTree big_tree(big_vocabulary);
                HuffTree worst_tree(worst_vocabulary);

                CHECK_NOTHROW(empty_archiver.encode(empty_tree));
                CHECK_NOTHROW(normal_archiver.encode(normal_tree));
                CHECK_NOTHROW(one_letter_archiver.encode(one_letter_tree));
                CHECK_NOTHROW(spaces_archiver.encode(spaces_tree));
                CHECK_NOTHROW(big_archiver.encode(big_tree));
                CHECK_NOTHROW(worst_archiver.encode(worst_tree));
                CHECK_EQ(empty_archiver._out.tellp(), 0);
                CHECK_EQ(normal_archiver._out.tellp(), 2);
                CHECK_EQ(one_letter_archiver._out.tellp(), 13);
                CHECK_EQ(spaces_archiver._out.tellp(), 5);
                CHECK(big_archiver._out.tellp() < 2000000);
                CHECK_EQ(worst_archiver._out.tellp(), 5000000);
            }

            SUBCASE("zip") {
                CHECK_NOTHROW(empty_archiver.zip());
                CHECK_NOTHROW(normal_archiver.zip());
                CHECK_NOTHROW(one_letter_archiver.zip());
                CHECK_NOTHROW(spaces_archiver.zip());
                CHECK_NOTHROW(big_archiver.zip());
                CHECK_NOTHROW(worst_archiver.zip());
                CHECK_EQ(empty_archiver._in_file_size, 0);
                CHECK_EQ(normal_archiver._in_file_size, 6);
                CHECK_EQ(one_letter_archiver._in_file_size, 100);
                CHECK_EQ(spaces_archiver._in_file_size, 25);
                CHECK(big_archiver._in_file_size > 3000000);
                CHECK_EQ(worst_archiver._in_file_size, 5000000);
                CHECK_EQ(empty_archiver._out_file_size, 0);
                CHECK_EQ(normal_archiver._out_file_size, 2);
                CHECK_EQ(one_letter_archiver._out_file_size, 13);
                CHECK_EQ(spaces_archiver._out_file_size, 5);
                CHECK(big_archiver._out_file_size < 2000000);
                CHECK_EQ(worst_archiver._out_file_size, 5000000);
                CHECK_EQ(empty_archiver._extra_data_size,
                         int(empty_archiver._out.tellp()) - empty_archiver._out_file_size);
                CHECK_EQ(normal_archiver._extra_data_size,
                         int(normal_archiver._out.tellp()) - normal_archiver._out_file_size);
                CHECK_EQ(one_letter_archiver._extra_data_size,
                         int(one_letter_archiver._out.tellp()) - one_letter_archiver._out_file_size);
                CHECK_EQ(spaces_archiver._extra_data_size,
                         int(spaces_archiver._out.tellp()) - spaces_archiver._out_file_size);
                CHECK_EQ(big_archiver._extra_data_size,
                         int(big_archiver._out.tellp()) - big_archiver._out_file_size);
                CHECK_EQ(worst_archiver._extra_data_size,
                         int(worst_archiver._out.tellp()) - worst_archiver._out_file_size);
            }
        }

        SUBCASE("unzip mode") {
            HuffmanArchiver zip_empty_archiver(empty_file, zip_empty_file);
            HuffmanArchiver zip_normal_archiver(normal_file, zip_normal_file);
            HuffmanArchiver zip_one_letter_archiver(one_letter_file, zip_one_letter_file);
            HuffmanArchiver zip_spaces_archiver(spaces_file, zip_spaces_file);
            HuffmanArchiver zip_big_archiver(big_file, zip_big_file);
            HuffmanArchiver zip_worst_archiver(worst_file, zip_worst_file);
            zip_empty_archiver.zip();
            zip_normal_archiver.zip();
            zip_one_letter_archiver.zip();
            zip_spaces_archiver.zip();
            zip_big_archiver.zip();
            zip_worst_archiver.zip();
            HuffmanArchiver empty_archiver(zip_empty_file, unzip_empty_file);
            HuffmanArchiver normal_archiver(zip_normal_file, unzip_normal_file);
            HuffmanArchiver one_letter_archiver(zip_one_letter_file, unzip_one_letter_file);
            HuffmanArchiver spaces_archiver(zip_spaces_file, unzip_spaces_file);
            HuffmanArchiver big_archiver(zip_big_file, unzip_big_file);
            HuffmanArchiver worst_archiver(zip_worst_file, unzip_worst_file);

            SUBCASE("extract_vocabulary") {
                std::array<uint32_t, UCHAR_MAX + 1> empty_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> normal_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> one_letter_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> spaces_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> big_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> worst_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> expected_empty_vocabulary{};
                std::array<uint32_t, UCHAR_MAX + 1> expected_normal_vocabulary{};
                expected_normal_vocabulary['a'] = 1;
                expected_normal_vocabulary['b'] = 1;
                expected_normal_vocabulary['c'] = 1;
                expected_normal_vocabulary['d'] = 1;
                expected_normal_vocabulary['e'] = 1;
                expected_normal_vocabulary['f'] = 1;
                std::array<uint32_t, UCHAR_MAX + 1> expected_one_letter_vocabulary{};
                expected_one_letter_vocabulary['a'] = 100;
                std::array<uint32_t, UCHAR_MAX + 1> expected_spaces_vocabulary{};
                expected_spaces_vocabulary[' '] = 10;
                expected_spaces_vocabulary['\n'] = 8;
                expected_spaces_vocabulary['a'] = 7;
                uint32_t tmp;
                empty_archiver._in.read((char *)&tmp, sizeof(tmp));
                normal_archiver._in.read((char *)&tmp, sizeof(tmp));
                one_letter_archiver._in.read((char *)&tmp, sizeof(tmp));
                spaces_archiver._in.read((char *)&tmp, sizeof(tmp));
                big_archiver._in.read((char *)&tmp, sizeof(tmp));
                worst_archiver._in.read((char *)&tmp, sizeof(tmp));
                REQUIRE(empty_archiver._in);
                REQUIRE(normal_archiver._in);
                REQUIRE(one_letter_archiver._in);
                REQUIRE(spaces_archiver._in);
                REQUIRE(big_archiver._in);
                REQUIRE(worst_archiver._in);

                CHECK_NOTHROW(empty_vocabulary = empty_archiver.extract_vocabulary());
                CHECK_NOTHROW(normal_vocabulary = normal_archiver.extract_vocabulary());
                CHECK_NOTHROW(one_letter_vocabulary = one_letter_archiver.extract_vocabulary());
                CHECK_NOTHROW(spaces_vocabulary = spaces_archiver.extract_vocabulary());
                CHECK_NOTHROW(big_vocabulary = big_archiver.extract_vocabulary());
                CHECK_NOTHROW(worst_vocabulary = worst_archiver.extract_vocabulary());
                CHECK_EQ(empty_vocabulary, expected_empty_vocabulary);
                CHECK_EQ(normal_vocabulary, expected_normal_vocabulary);
                CHECK_EQ(one_letter_vocabulary, expected_one_letter_vocabulary);
                CHECK_EQ(spaces_vocabulary, expected_spaces_vocabulary);
            }

            SUBCASE("fill_buffer") {
                normal_archiver._in = std::ifstream(normal_file, std::ios_base::binary);
                normal_archiver._in.exceptions(std::ios_base::badbit | std::ios_base::failbit);
                empty_archiver._in = std::ifstream(empty_file, std::ios_base::binary);
                empty_archiver._in.exceptions(std::ios_base::badbit | std::ios_base::failbit);
                std::queue<bool> buffer;

                CHECK_NOTHROW(normal_archiver.fill_buffer(buffer));
                CHECK_EQ(buffer.size(), 8);
                for (int i = 0; i < 8; ++i) {
                    if (i == 0 || i == 5 || i == 6) {
                        CHECK(buffer.front());
                    } else {
                        CHECK(!buffer.front());
                    }
                    buffer.pop();
                }
                CHECK_THROWS(empty_archiver.fill_buffer(buffer));
            }

            SUBCASE("decode") {
                uint32_t tmp;
                empty_archiver._in.read((char *)&empty_archiver._out_file_size,
                                        sizeof(empty_archiver._out_file_size));
                normal_archiver._in.read((char *)&normal_archiver._out_file_size,
                                         sizeof(normal_archiver._out_file_size));
                one_letter_archiver._in.read((char *)&one_letter_archiver._out_file_size,
                                             sizeof(one_letter_archiver._out_file_size));
                spaces_archiver._in.read((char *)&spaces_archiver._out_file_size,
                                         sizeof(spaces_archiver._out_file_size));
                big_archiver._in.read((char *)&big_archiver._out_file_size,
                                      sizeof(big_archiver._out_file_size));
                worst_archiver._in.read((char *)&worst_archiver._out_file_size,
                                        sizeof(worst_archiver._out_file_size));
                REQUIRE_EQ(empty_archiver._out_file_size, 0);
                REQUIRE_EQ(normal_archiver._out_file_size, 6);
                REQUIRE_EQ(one_letter_archiver._out_file_size, 100);
                REQUIRE_EQ(spaces_archiver._out_file_size, 25);
                REQUIRE(big_archiver._out_file_size > 3000000);
                REQUIRE_EQ(worst_archiver._out_file_size, 5000000);
                std::array<uint32_t, UCHAR_MAX + 1> empty_vocabulary = empty_archiver.extract_vocabulary();
                std::array<uint32_t, UCHAR_MAX + 1> normal_vocabulary = normal_archiver.extract_vocabulary();
                std::array<uint32_t, UCHAR_MAX + 1> one_letter_vocabulary = one_letter_archiver.extract_vocabulary();
                std::array<uint32_t, UCHAR_MAX + 1> spaces_vocabulary = spaces_archiver.extract_vocabulary();
                std::array<uint32_t, UCHAR_MAX + 1> big_vocabulary = big_archiver.extract_vocabulary();
                std::array<uint32_t, UCHAR_MAX + 1> worst_vocabulary = worst_archiver.extract_vocabulary();
                HuffTree empty_tree(empty_vocabulary);
                HuffTree normal_tree(normal_vocabulary);
                HuffTree one_letter_tree(one_letter_vocabulary);
                HuffTree spaces_tree(spaces_vocabulary);
                HuffTree big_tree(big_vocabulary);
                HuffTree worst_tree(worst_vocabulary);

                CHECK_NOTHROW(empty_archiver.decode(empty_tree));
                CHECK_NOTHROW(normal_archiver.decode(normal_tree));
                CHECK_NOTHROW(one_letter_archiver.decode(one_letter_tree));
                CHECK_NOTHROW(spaces_archiver.decode(spaces_tree));
                CHECK_NOTHROW(big_archiver.decode(big_tree));
                CHECK_NOTHROW(worst_archiver.decode(worst_tree));
                CHECK_EQ(empty_archiver._out.tellp(), 0);
                CHECK_EQ(normal_archiver._out.tellp(), 6);
                CHECK_EQ(one_letter_archiver._out.tellp(), 100);
                CHECK_EQ(spaces_archiver._out.tellp(), 25);
                CHECK(big_archiver._out.tellp() > 3000000);
                CHECK_EQ(worst_archiver._out.tellp(), 5000000);
            }

            SUBCASE("unzip") {
                CHECK_NOTHROW(empty_archiver.unzip());
                CHECK_NOTHROW(normal_archiver.unzip());
                CHECK_NOTHROW(one_letter_archiver.unzip());
                CHECK_NOTHROW(spaces_archiver.unzip());
                CHECK_NOTHROW(big_archiver.unzip());
                CHECK_NOTHROW(worst_archiver.unzip());
                CHECK_EQ(empty_archiver._in_file_size, 0);
                CHECK_EQ(normal_archiver._in_file_size, 2);
                CHECK_EQ(one_letter_archiver._in_file_size, 13);
                CHECK_EQ(spaces_archiver._in_file_size, 5);
                CHECK(big_archiver._in_file_size < 2000000);
                CHECK_EQ(worst_archiver._in_file_size, 5000000);
                CHECK_EQ(empty_archiver._out_file_size, 0);
                CHECK_EQ(normal_archiver._out_file_size, 6);
                CHECK_EQ(one_letter_archiver._out_file_size, 100);
                CHECK_EQ(spaces_archiver._out_file_size, 25);
                CHECK(big_archiver._out_file_size > 3000000);
                CHECK_EQ(worst_archiver._out_file_size, 5000000);
                CHECK_EQ(empty_archiver._extra_data_size,
                         int(empty_archiver._in.tellg()) - empty_archiver._in_file_size);
                CHECK_EQ(normal_archiver._extra_data_size,
                         int(normal_archiver._in.tellg()) - normal_archiver._in_file_size);
                CHECK_EQ(one_letter_archiver._extra_data_size,
                         int(one_letter_archiver._in.tellg()) - one_letter_archiver._in_file_size);
                CHECK_EQ(spaces_archiver._extra_data_size,
                         int(spaces_archiver._in.tellg()) - spaces_archiver._in_file_size);
                CHECK_EQ(big_archiver._extra_data_size,
                         int(big_archiver._in.tellg()) - big_archiver._in_file_size);
                CHECK_EQ(worst_archiver._extra_data_size,
                         int(worst_archiver._in.tellg()) - worst_archiver._in_file_size);
            }
        }

        SUBCASE("zip and unzip") {
            HuffmanArchiver zip_empty_archiver(empty_file, zip_empty_file);
            HuffmanArchiver zip_normal_archiver(normal_file, zip_normal_file);
            HuffmanArchiver zip_one_letter_archiver(one_letter_file, zip_one_letter_file);
            HuffmanArchiver zip_spaces_archiver(spaces_file, zip_spaces_file);
            HuffmanArchiver zip_big_archiver(big_file, zip_big_file);
            HuffmanArchiver zip_worst_archiver(worst_file, zip_worst_file);
            zip_empty_archiver.zip();
            zip_normal_archiver.zip();
            zip_one_letter_archiver.zip();
            zip_spaces_archiver.zip();
            zip_big_archiver.zip();
            zip_worst_archiver.zip();
            HuffmanArchiver unzip_empty_archiver(zip_empty_file, unzip_empty_file);
            HuffmanArchiver unzip_normal_archiver(zip_normal_file, unzip_normal_file);
            HuffmanArchiver unzip_one_letter_archiver(zip_one_letter_file, unzip_one_letter_file);
            HuffmanArchiver unzip_spaces_archiver(zip_spaces_file, unzip_spaces_file);
            HuffmanArchiver unzip_big_archiver(zip_big_file, unzip_big_file);
            HuffmanArchiver unzip_worst_archiver(zip_worst_file, unzip_worst_file);
            unzip_empty_archiver.unzip();
            unzip_normal_archiver.unzip();
            unzip_one_letter_archiver.unzip();
            unzip_spaces_archiver.unzip();
            unzip_big_archiver.unzip();
            unzip_worst_archiver.unzip();

            CHECK(compare_files(empty_file, unzip_empty_file));
            CHECK(compare_files(normal_file, unzip_normal_file));
            CHECK(compare_files(one_letter_file, unzip_one_letter_file));
            CHECK(compare_files(spaces_file, unzip_spaces_file));
            CHECK(compare_files(big_file, unzip_big_file));
            CHECK(compare_files(worst_file, unzip_worst_file));
        }
    }

    static bool compare_files(const std::string &file1, const std::string &file2) {
        std::ifstream in1(file1, std::ios_base::binary);
        std::ifstream in2(file2, std::ios_base::binary);
        if (!in1) {
            throw std::invalid_argument("Couldn't open file \"" + file1 + "\".");
        }
        if (!in2) {
            throw std::invalid_argument("Couldn't open file \"" + file2 + "\".");
        }
        char chr1 = 0, chr2 = 0;
        do {
            if (chr1 != chr2) {
                return false;
            }
            in1.read(&chr1, sizeof(chr1));
            in2.read(&chr2, sizeof(chr2));
        } while (in1 && in2);
        if (in1 || in2) {
            return false;
        }
        return true;
    }
};

TEST_CASE("zip time limit" * doctest::timeout(5)) {
    std::string worst_file = path("worst.txt");
    std::string zip_worst_file = path("zip worst.txt");
    HuffmanArchiver worst_archiver(worst_file, zip_worst_file);
    worst_archiver.zip();
}

TEST_CASE("unzip time limit" * doctest::timeout(5)) {
    std::string zip_worst_file = path("zip worst.txt");
    std::string unzip_worst_file = path("unzip worst.txt");
    HuffmanArchiver worst_archiver(zip_worst_file, unzip_worst_file);
    worst_archiver.unzip();
}
