from balltree import PyBalltree, PyQuery, balltree_search, balltree_search_with_dp, linear_search, linear_search_with_dp

ivector_list = [[2.8, 3.9], [2.1, 2.7], [2.8, 3.1], [3., 2.8],
                [3.1, 3.], [2.6, 9.1], [3.5, 9.2], [3.1, 8.6],
                [3.6, 8.8], [8.2, 7.6], [9.2, 8.5], [9.3, 7.5],
                [8.3, 6.3], [8., 6.], [8.4, 6.1], [9., 6.4],
                [9.4, 6.8], [9.2, 6.6], [9.1, 6.1], [7.9, 3.7],
                [8.8, 3.2], [9.1, 2.7], [8.7, 1.8], [8.9, 1.5]]

stree = PyBalltree(ivector_list)
stree.build()
stree.pickle('tmp2.bt')

stree2 = PyBalltree(ivector_list)
stree2.build_from_file('tmp2.bt')

vec = [1., 2.]
topk = 7
black_ids = []
white_ids = []

q = PyQuery(vec, topk, black_ids, white_ids)
answer = balltree_search(q, stree)
print answer

answer2 = balltree_search(q, stree2)
print answer2

answer3 = balltree_search_with_dp(q, stree2)
print answer3

answer4 = linear_search(q, ivector_list)
print answer4

answer5 = linear_search_with_dp(q, ivector_list)
print answer5
