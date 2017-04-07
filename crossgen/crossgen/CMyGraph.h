#pragma once

#include<iostream>
#include<vector>
#include<list>
#include<set>
#include<map>
#include<iterator>

using namespace std;

template<class TVertexId, class TWeight = int, int isEasy = 1> class CMyGraph {
public:
	CMyGraph();

	void AddVertex(TVertexId);
	void AddEdge(TVertexId, TVertexId, TWeight);

	class CVertexIterator {
	public:
		CVertexIterator() {}
		CVertexIterator(const CVertexIterator& iter) { it = iter.it; }
		CVertexIterator(CVertexIterator&& iter) { it = iter.it; }

		CVertexIterator& operator++(int) { it++; return *this; }
		CVertexIterator& operator--(int) { it--; return *this; }
		const TVertexId& operator*() const { return *it; }
		bool operator==(const CVertexIterator& iter) const { return it == iter.it; }
		bool operator!=(const CVertexIterator& iter) const { return it != iter.it; }
		CVertexIterator& operator=(const CVertexIterator& iter) { it = iter.it; return *this; }
		CVertexIterator& operator=(CVertexIterator&& iter) { it = iter.it; return *this; }

		bool IsValid() const { return it.valid(); }

		//нужен для операторов begin&end
		CVertexIterator(typename set<TVertexId>::iterator iter) {
			it = iter;
		}
	private:
		typename set<TVertexId>::const_iterator it;
	};

	class CEdgeIterator {
	public:
		CEdgeIterator() {}
		CEdgeIterator(const CEdgeIterator& iter) { it = iter.it; sourceVertex = iter.sourceVertex; }
		CEdgeIterator(CEdgeIterator&& iter) { it = iter.it; sourceVertex = iter.sourceVertex; }

		CEdgeIterator& operator++(int) { it++; return *this; }
		CEdgeIterator& operator--(int) { it--; return *this; }
		bool operator==(const CEdgeIterator& iter) const { return (it == iter.it) && (sourceVertex == iter.sourceVertex); }
		bool operator!=(const CEdgeIterator& iter) const { return (it != iter.it) || (sourceVertex != iter.sourceVertex); }
		CEdgeIterator& operator=(const CEdgeIterator& iter) { it = iter.it; sourceVertex = iter.sourceVertex; return *this; }
		CEdgeIterator& operator=(CEdgeIterator&& iter) { it = iter.it; sourceVertex = iter.sourceVertex; return *this; }
		const TVertexId& GetSourceVertex() const { return sourceVertex; }
		const TVertexId& GetTargetVertex() const { return (*it).first; }
		const TWeight& GetWeight() const { return (*it).second; }
		//IsValid не работает
		bool IsValid() const { return it.IsValid(); }

		CEdgeIterator(typename list<pair<TVertexId, TWeight>>::iterator iter, TVertexId vert) { it = iter; sourceVertex = vert; }
		CEdgeIterator(typename list<pair<TVertexId, TWeight>>::const_iterator iter, TVertexId vert) { it = iter; sourceVertex = vert; }
		operator typename list<pair<TVertexId, TWeight>>::iterator() {
			return it;
		}
	private:
		typename list<pair<TVertexId, TWeight>>::const_iterator it;
		TVertexId sourceVertex;
	};
	CVertexIterator Begin() const { return vertexes.begin(); }
	CVertexIterator End() const { return vertexes.end(); }
	CVertexIterator FindVertex(const TVertexId& vert) const { return vertexes.find(vert); }
	void DeleteVertex(CVertexIterator&);
	void DeleteEdge(CEdgeIterator&);

	CEdgeIterator OutgoingEdgesBegin(const CVertexIterator& it) const { return CEdgeIterator((*edges.find(*it)).second.begin(), *it); }
	CEdgeIterator OutgoingEdgesEnd(const CVertexIterator& it) const { return CEdgeIterator((*edges.find(*it)).second.end(), *it); }

	typename CMyGraph<TVertexId, TWeight, isEasy> Transpose();

	//косарайю
	map<TVertexId, int> Kosarayu();
	//тарьяно
	map<TVertexId, int> Taryano();
	//у них dfs стрёмные, ибо много всего передаётся в аргументах. если же запихивать всё это в приватные поля графа выйдет много лишнего
	map<TVertexId, TWeight> BellmanFord(TVertexId);

	int size() {
		return vertexes.size();
	}
private:
	set<TVertexId> vertexes;
	map<TVertexId, list<pair<TVertexId, TWeight>>> edges;
	void dfs1_Kosarayu(TVertexId vert, map<TVertexId, char>& colors, vector<TVertexId>& order) {
		colors[vert] = 0;
		for (auto it = OutgoingEdgesBegin(FindVertex(vert)); it != OutgoingEdgesEnd(FindVertex(vert)); it++) {
			if (colors[it.GetTargetVertex()] == -1) dfs1_Kosarayu(it.GetTargetVertex(), colors, order);
		}
		order.push_back(vert);
		colors[vert] = 1;
	}
	void dfs2_Kosarsyu(TVertexId vert, map<TVertexId, int>& components, int nowCompId) {
		components[vert] = nowCompId;
		for (auto it = OutgoingEdgesBegin(FindVertex(vert)); it != OutgoingEdgesEnd(FindVertex(vert)); it++) {
			if (components[it.GetTargetVertex()] == -1) {
				components[it.GetTargetVertex()] = nowCompId;
				dfs2_Kosarsyu(it.GetTargetVertex(), components, nowCompId);
			}
		}
	}
	void dfs_Taryan(TVertexId u, int& time, map<TVertexId, int>& lowlink, map<TVertexId, bool>& used, vector<TVertexId>& stack, int& nowCompId,
		map<TVertexId, int>& components) {
		lowlink[u] = time++;
		used[u] = true;
		stack.push_back(u);
		bool isRoot = true;

		for (auto it = OutgoingEdgesBegin(FindVertex(u)); it != OutgoingEdgesEnd(FindVertex(u)); it++) {
			if (!used[it.GetTargetVertex()])
				dfs_Taryan(it.GetTargetVertex(), time, lowlink, used, stack, nowCompId, components);
			if (lowlink[u] > lowlink[it.GetTargetVertex()]) {
				lowlink[u] = lowlink[it.GetTargetVertex()];
				isRoot = false;
			}
		}

		if (isRoot) {
			while (true) {
				TVertexId k = stack[stack.size() - 1];
				stack.pop_back();
				components[k] = nowCompId;
				lowlink[k] = 1000000000;//очень большое число
				if (k == u) {
					nowCompId++;
					break;
				}
			}
		}
	}
};

template<class TVertexId, class TWeight, int isEasy>
CMyGraph<TVertexId, TWeight, isEasy>::CMyGraph() {

}

template<class TVertexId, class TWeight, int isEasy>
void CMyGraph<TVertexId, TWeight, isEasy>::AddVertex(TVertexId v) {
	if (vertexes.empty() || (vertexes.find(v) == vertexes.end())) {
		vertexes.insert(v);
		edges.insert(make_pair(v, list<pair<TVertexId, TWeight>>()));
	}
	else cerr << "Error! This vertex is already in graph!" << endl;
}

template<class TVertexId, class TWeight, int isEasy>
void CMyGraph<TVertexId, TWeight, isEasy>::AddEdge(TVertexId from, TVertexId to, TWeight weight) {
	if ((FindVertex(from) == End()) || (FindVertex(to) == End())) {
		cerr << "Error! Edge will not be added, because source vertex doesn't exist!" << endl;
		return;
	}

	//блок проверки на гиперграф
	if (isEasy) {
		for (auto iter = edges[from].begin(); iter != edges[from].end(); iter++) {
			if ((*iter).first == to) {
				cerr << "Error! This edge is already exists! It is not gipergraph!" << endl;
				return;
			}
		}
	}

	edges[from].push_back(make_pair(to, weight));
}

template<class TVertexId, class TWeight, int isEasy>
void CMyGraph<TVertexId, TWeight, isEasy>::DeleteVertex(CVertexIterator& vertIterator) {
	for (auto it = edges.begin(); it != edges.end(); it++) {
		if ((*it).first != *vertIterator) {
			for (auto iter = (*it).second.begin(); iter != (*it).second.end(); iter++) {
				if ((*iter).first == *vertIterator) {
					(*it).second.erase(iter);
					break;
				}
			}
		}
	}
	edges.erase(*vertIterator);
	vertexes.erase(*vertIterator);
}

template<class TVertexId, class TWeight, int isEasy>
void CMyGraph<TVertexId, TWeight, isEasy>::DeleteEdge(CEdgeIterator& edgeIterator) {
	edges[edgeIterator.GetSourceVertex()].erase(edgeIterator);
}

template<class TVertexId, class TWeight, int isEasy>
typename CMyGraph<TVertexId, TWeight, isEasy> CMyGraph<TVertexId, TWeight, isEasy>::Transpose() {
	CMyGraph<TVertexId, TWeight, isEasy> swp;
	for (auto i = Begin(); i != End(); i++) {
		swp.AddVertex(*i);
	}
	for (auto i = Begin(); i != End(); i++) {
		for (auto j = edges[*i].begin(); j != edges[*i].end(); j++) {
			swp.AddEdge((*j).first, *i, (*j).second);
		}
	}
	return swp;
}

template<class TVertexId, class TWeight, int isEasy>
map<TVertexId, int> CMyGraph<TVertexId, TWeight, isEasy>::Kosarayu() {
	map<TVertexId, char> colors;//-1 white in dfs, 0 - grey, 1 - black
	for (auto it = Begin(); it != End(); it++) colors.insert(make_pair(*it, -1));
	vector<TVertexId> order;//vertexes sorted by time of outgoing from dfs
	for (auto it = Begin(); it != End(); it++) {
		if (colors[*it] == -1) dfs1_Kosarayu(*it, colors, order);
	}
	//now we have order
	map<TVertexId, int> components;//our future result
	for (auto it = Begin(); it != End(); it++) components.insert(make_pair(*it, -1));

	CMyGraph<TVertexId, TWeight> transp = Transpose();
	int nowCompId = 0;
	for (int i = order.size() - 1; i >= 0; i--) {
		if (components[order[i]] == -1) {
			nowCompId++;
			transp.dfs2_Kosarsyu(order[i], components, nowCompId);
		}
	}
	return components;
}

template<class TVertexId, class TWeight, int isEasy>
map<TVertexId, int> CMyGraph<TVertexId, TWeight, isEasy>::Taryano() {
	int n = vertexes.size();
	map<TVertexId, int> lowlink;
	for (auto it = Begin(); it != End(); it++) lowlink[*it] = 0;
	map<TVertexId, bool> used;
	for (auto it = Begin(); it != End(); it++) used[*it] = false;
	vector<int> stack;
	map<TVertexId, int> components;
	int time = 0;
	int nowCompId = 1;
	for (auto it = Begin(); it != End(); it++)
		if (!used[*it])
			dfs_Taryan(*it, time, lowlink, used, stack, nowCompId, components);

	return components;
}

template<class TVertexId, class TWeight, int isEasy>
map<TVertexId, TWeight> CMyGraph<TVertexId, TWeight, isEasy>::BellmanFord(TVertexId vert) {
	map<TVertexId, TWeight> result;
	map<TVertexId, bool> wasReached;
	for (auto it = Begin(); it != End(); it++) wasReached[*it] = false;
	result[vert] = 0;//вообще может быть и не ноль, зависит от задаче, например, в арбитраже 1
	wasReached[vert] = true;
	for (size_t i = 0; i < vertexes.size(); i++) {
		for (auto it = Begin(); it != End(); it++) {
			if (wasReached[*it]) for (auto edgeIt = OutgoingEdgesBegin(it); edgeIt != OutgoingEdgesEnd(it); edgeIt++) {
				if (!wasReached[edgeIt.GetTargetVertex()]) {
					wasReached[edgeIt.GetTargetVertex()] = true;
					result[edgeIt.GetTargetVertex()] = result[edgeIt.GetSourceVertex()] + edgeIt.GetWeight();
				}
				else {
					if (result[edgeIt.GetTargetVertex()] > result[edgeIt.GetSourceVertex()] + edgeIt.GetWeight())
						result[edgeIt.GetTargetVertex()] = result[edgeIt.GetSourceVertex()] + edgeIt.GetWeight();
				}
			}
		}
	}
	////чтоб зашло на контест
	//for (auto it = Begin(); it != End(); it++){
	//	if (!wasReached[*it]) result[*it] = 30000;
	//}
	return result;
}

//сравнивает вывод тарьяно и косарайю
template<typename TVertexId>
bool compareCompanents(map<TVertexId, int>& first, map<TVertexId, int>& second) {
	set<int> was;
	set<int> were;

	for (auto it = first.begin(); it != first.end(); it++) {
		int comp_number = (*it).second;
		if ((was.find(comp_number) == was.end()) || (was.size() == 0)) {
			was.insert(comp_number);
			if ((were.find(second[(*it).first]) != were.end()) && (were.size() != 0)) return false;
			were.insert(second[(*it).first]);
			int y = second[(*it).first];

			auto jt = it;
			jt++;
			for (; jt != first.end(); jt++) {
				if ((*jt).second == comp_number) {
					if (second[(*jt).first] != y) return false;
				}
			}
		}
	}
	return true;
}