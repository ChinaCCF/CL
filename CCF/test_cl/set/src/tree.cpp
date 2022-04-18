#include <libCL/set/6rbtree.h>
#include <libCL/set/2arr.h>

namespace test
{
	namespace ntest_rbtree
	{

	}

	void test_rbtree()
	{
		printf("test_rbtree\n");

		{
			cl::RBTree<int, int> map;
		}
		
		{
			cl::RBTree<int, int> map;
			map[1] = 1;
			map[2] = 2;

			CL_Assert(map.size() == 2);

			for (auto& p : map)
			{
				printf("%d : %d\n", p.first, p.second);
				CL_Assert(p.first == p.second);
			}

			int i = 0;
		}

		{
			cl::RBTree<int, int> map;
			map[1] = 1;
			map[2] = 2;
			map[3] = 3;
			map[4] = 4;
			map[5] = 5;
			map[6] = 6;
			map[7] = 7;
			map[8] = 8;
			map[9] = 9;
			map[10] = 10;
			map[11] = 11;
			map[12] = 12;
			map[13] = 13;
			map[14] = 14;
			map[15] = 15;
			map[16] = 16;
			map[17] = 17;

			CL_Assert(map.size() == 17);

			for (auto& p : map) 
				CL_Assert(p.first == p.second); 

			map.remove(10);
			CL_Assert(map.size() == 16);
			for (auto& p : map)
				CL_Assert(p.first == p.second);

			auto it = map.find(12);
			map.remove(it);
			CL_Assert(map.size() == 15);
			for (auto& p : map)
				CL_Assert(p.first == p.second);

			int i = 0;
		}

		{
			cl::RBTree<int, int> map;
			map[1] = 1;
			map[2] = 2;
			map[3] = 3;
			map[4] = 4;
			map[5] = 5;
			map[6] = 6;
			map[7] = 7;
			map[8] = 8;
			map[9] = 9;
			map[10] = 10;
			map[11] = 11; 

			cl::RBTree<int, int> map2;  
			map2[10] = 11;
			map2[11] = 12;
			map2[12] = 13;
			map2[13] = 14;
			map2[14] = 15;
			map2[15] = 16;
			map2[16] = 17;
			map2[17] = 18;

			map << map2;

			CL_Assert(map.size() == 17);

			for (auto& p : map)
			{
				if(p.first < 10)
				CL_Assert(p.first == p.second);
				else
					CL_Assert(p.first == (p.second - 1));
			}
				 
			int i = 0;
		}
		{
			cl::RBTree<int, int> map;
			map[1] = 1;
			map[2] = 2;
			map[3] = 3;
			map[4] = 4;
			map[5] = 5;
			map[6] = 6;
			map[7] = 7;
			map[8] = 8;
			map[9] = 9; 

			cl::RBTree<int, int> map2;
			map2[10] = 10;
			map2[11] = 11;
			map2[12] = 12;
			map2[13] = 13;
			map2[14] = 14;
			map2[15] = 15;
			map2[16] = 16;
			map2[17] = 17;
			
			map << std::move(map2);

			CL_Assert(map.size() == 17);
			CL_Assert(map2.size() == 0);

			for (auto& p : map)
				CL_Assert(p.first == p.second);

			for (auto& p : map2)
				CL_Assert(p.first == p.second);
			 
			int i = 0;
		}
		{
			cl::RBTree<int, int> map;
			map[1] = 1;
			map[2] = 2;
			map[3] = 3;
			map[4] = 4;
			map[5] = 5;
			map[6] = 6;
			map[7] = 7;
			map[8] = 8;
			map[9] = 9;

			cl::RBTree<int, int> map2 = std::move(map);
			 
			CL_Assert(map.size() == 0);
			CL_Assert(map2.size() == 9);

			for (auto& p : map2)
				CL_Assert(p.first == p.second);
			  
			int i = 0;
		}

		{
			cl::RBTree<int, int> map;
			map[275] = 275; 
			map[711] = 711;
			map[260] = 260; 
			map[515] = 515;
			map[442] = 442;
			map[800] = 800;
			map[900] = 900;
			map[50] = 50;
			map[270] = 270;
			map[20] = 20;
			map[30] = 30;

			//20  30  50  260  270  275  442  515  711  800  900 
			CL_Assert(map.size() == 11); 
			 
			cl::Stack<int, 32> stack;

			for (auto p : map)
				stack.push(p.first);

			CL_Assert(stack[0] == 20);
			CL_Assert(stack[1] == 30);
			CL_Assert(stack[2] == 50);
			CL_Assert(stack[3] == 260);
			CL_Assert(stack[4] == 270);
			CL_Assert(stack[5] == 275);
			CL_Assert(stack[6] == 442);
			CL_Assert(stack[7] == 515);
			CL_Assert(stack[8] == 711);
			CL_Assert(stack[9] == 800);
			CL_Assert(stack[10] == 900);

			auto it = map.find(20);
			CL_Assert(it.node_->is_black() == false);

			it = map.find(30);
			CL_Assert(it.node_->is_black() == true);

			it = map.find(50);
			CL_Assert(it.node_->is_black() == false);

			it = map.find(260);
			CL_Assert(it.node_->is_black() == false);

			it = map.find(270);
			CL_Assert(it.node_->is_black() == true);

			it = map.find(275);
			CL_Assert(it.node_->is_black() == true);

			it = map.find(515);
			CL_Assert(it.node_->is_black() == false);

			it = map.find(442);
			CL_Assert(it.node_->is_black() == true);

			it = map.find(800);
			CL_Assert(it.node_->is_black() == true);

			it = map.find(711);
			CL_Assert(it.node_->is_black() == false);

			it = map.find(900);
			CL_Assert(it.node_->is_black() == false);
			int i = 0;
		}
	}
}