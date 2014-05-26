//k-d tree implementation
#include <array>
#include <vector>

//value type, number of dimensions, max bucket size, squared boundry 0 for unsymetric structure,
template<class DATATYPE, int NDIMS,int NMAXLEAVES,int ROOTSIZE,int NODEBOUNDRY>
class kdtree{
private:
	bool leaf = true;
	int di;
	std::array<int,NDIMS> depth;
	std::vector<DATATYPE*> bucket;
	DATATYPE dims;
	kdtree * Left, *Right;
public:
	kdtree(){};
	kdtree(DATATYPE dt, std::array<int, NDIMS> ndepth, int dim) : dims(dt), depth(ndepth)
	{
		depth[dim]++;
		di = (dim + 1 >= NDIMS ? 0 : dim + 1);
	};
	void insert(DATATYPE * nvalue){
		//if leaf insert value here
		if (leaf){
			bucket.push_back(nvalue);
			if (bucket.size() >= NMAXLEAVES){
				leaf = false;
				split();
			}
		}
		//else insert into child node
		else{		
				if ((*nvalue)[di] >= dims[di]){
					Right->insert(nvalue);
				}
				else{
					Left->insert(nvalue);
				}
		}
	}
	std::vector<DATATYPE*> getbucket(DATATYPE nvalue){
		if (leaf){
			return bucket;
		}
		else{
			//if the point is too close the the boundry, search both sides
			if (abs(dims[di] - nvalue[di]) <= NODEBOUNDRY){
				std::vector<DATATYPE*> a,b;
				a = Right->getbucket(nvalue);
				b = Left->getbucket(nvalue);
				//merge resulting arrays
				a.insert(a.end(), b.begin(), b.end());
				return a;
			}
			else{
				if (nvalue[di] >= dims[di]){
					return Right->getbucket(nvalue);
				}
				else{
					return Left->getbucket(nvalue);
				}
			}
		}
	}
	
	//split a leaf into a branch
	void split(){
		float w = ROOTSIZE / powf(2, depth[di]+1);
		DATATYPE dl = dims;
		dl[di] -= w;
		Left = new kdtree(dl, depth, di);

		DATATYPE dr = dims;
		dr[di] += w;
		Right = new kdtree(dr, depth, di);
		
		for (int i = 0; i < bucket.size(); i++){
			insert(bucket[i]);
		}
		bucket.clear();
	}
	void clear(){
		if (!leaf){
			Left->clear();
			Right->clear();
			delete Left;
			delete Right;
			leaf = true;
		}
		else{
			bucket.clear();
		}
	}
};