#include "StdAfx.h"
#include "BestWorstAntSystem.h"
#include "resource.h"
#include <limits>       // std::numeric_limits
#include "Utils.h"



CBestWorstAntSystem::CBestWorstAntSystem(Parameters& Par,MatrixArrayTypeInt *distanceMatrix) 
	: CAntSystem(Par , distanceMatrix)
{
	Restart =0;
	restart_iteration =1;
	starttime=GetTickCount();
	std::vector<bool> visited(m_noNodes);
	std::vector<size_t> nntour(m_noNodes);
	calculateNearestNeigbhor(20);
	//calculate min max values inital
	int phase = 0;
	int rnd= (rand()%(visited.size()-1))+1;
	nntour[0] =rnd;
	visited[rnd] = true;
	chooseClosestNext(visited, nntour);
	double distance = this->calculatePathLength( nntour);
	trail_0 = 1. /distance;

}
/*    
      FUNCTION:      uses additional evaporation on the arcs of iteration worst
                     ant that are not shared with the global best ant
      INPUT:         pointer to the worst (a1) and the best (a2) ant
      OUTPUT:        none
      (SIDE)EFFECTS: pheromones on some arcs undergo additional evaporation
*/
void CBestWorstAntSystem::bwas_worst_ant_update( const CAnt &worseA, const CAnt &bestA)
{  
    size_t   i, pos, pred;
    unsigned int   distance;
    
	std::vector<size_t> pos2(m_noNodes+1);

    for ( i = 0 ; i < bestA.getNoNodes() ; i++ ) 
	{
		pos2[bestA.getCity(i)] = i;
    }
 
    distance = 0;
    for ( i = 1 ; i < m_noNodes   ; i++ ) 
	{
		size_t to = worseA.getCity(i);
		size_t from = worseA.getCity(i-1);
		pos = pos2[to];

		if(pos+1 > m_noNodes)
			pred = m_noNodes - 1;
		else if(pos-1 < 0)
			pred = 0;
		else
			pred = pos;



		size_t bestnextpos = bestA.getCity(pred+1);
		size_t bestprevpos = bestA.getCity(pred-1);

		if (bestnextpos == from)
			; /* do nothing, edge is common with a2 (best solution found so far) */
		else if (bestprevpos == from)
			; /* do nothing, edge is common with a2 (best solution found so far) */
		else 
		{   /* edge (j,h) does not occur in ant a2 */       
			m_newPheromoneMatrix->evaporate1(to,from);
			m_newPheromoneMatrix->evaporate1(from,to);
			//(*m_pheromoneMatrix)[j][h] = (1 - m_rho) * (*m_pheromoneMatrix)[j][h];
			//(*m_pheromoneMatrix)[h][j] = (1 - m_rho) * (*m_pheromoneMatrix)[h][j];
		}
    }

}
long CBestWorstAntSystem::distance_between_ants( const CAnt &worseA, const CAnt &bestA)
{
	size_t   i, pos, pred;
	unsigned int   distance;

	std::vector<size_t> pos2(m_noNodes+1);



	for ( i = 0 ; i < bestA.getNoNodes() ; i++ ) 
	{
		pos2[bestA.getCity(i)] = i;
	}

	distance = 0;
	for ( i = 1 ; i < m_noNodes   ; i++ ) 
	{
		size_t to = worseA.getCity(i);
		size_t from = worseA.getCity(i-1);
		pos = pos2[to];

		if(pos+1 > m_noNodes)
			pred = m_noNodes - 1;
		else if(pos-1 < 0)
			pred = 0;
		else
			pred = pos;



		size_t bestnextpos = bestA.getCity(pred+1);
		size_t bestprevpos = bestA.getCity(pred-1);

		if (bestnextpos == from)
			; /* do nothing, edge is common with a2 (best solution found so far) */
		else if (bestprevpos == from)
			; /* do nothing, edge is common with a2 (best solution found so far) */
		else {   /* edge (j,h) does not occur in ant a2 */       
			distance++;

			//(*m_pheromoneMatrix)[j][h] = (1 - m_rho) * (*m_pheromoneMatrix)[j][h];
			//(*m_pheromoneMatrix)[h][j] = (1 - m_rho) * (*m_pheromoneMatrix)[h][j];
		}
	}
	return distance;
}
void CBestWorstAntSystem::chooseClosestNext(std::vector<bool> &antsvisted , std::vector<size_t> &nntour)
{
	size_t city, current_city, next_city;
	int min_distance;
	next_city =this->m_noNodes;

	for(size_t phase = 1; phase < m_noNodes; phase++)
	{
		current_city = nntour[phase - 1];
		min_distance = (std::numeric_limits<int>::max)();

		for (city = 0; city < m_noNodes; city++)
		{
			if (antsvisted[city])
				; /* city already visited */
			else
			{

				if ((*m_distanceMatrix)[current_city][city] < min_distance)
				{
					next_city = city;
					min_distance = (*m_distanceMatrix)[current_city][city];
				}
			}
		}


		nntour[phase] = next_city;
		antsvisted[next_city] = true;
	}

}
void CBestWorstAntSystem::calculateNearestNeigbhor(unsigned int NUMOFANTS)
{
	unsigned int i = 0;
	int *pHelpArray = new int[m_noNodes];
	double *pdistanceArray = new double[m_noNodes ];
	*(pdistanceArray) =(std::numeric_limits<double>::max)();

	m_nnList.resize(m_noNodes);
	for (unsigned int i = 0; i < m_noNodes; ++i)
		m_nnList[i].resize(NUMOFANTS);


	for (unsigned int node = 0; node < m_noNodes; node++) 
	{ 

		for (i = 0; i < m_noNodes; i++) 
		{
			*(pdistanceArray+i)=(*m_distanceMatrix)[node][i];
			*(pHelpArray+i)=  i;
		}
		double max = (std::numeric_limits<double>::max)() - 1; 
		*(pdistanceArray+node) =  (std::numeric_limits<double>::max)();  // set to a large value .. 
		this->m_pLocalSearch->sort2(pdistanceArray, pHelpArray, 0, static_cast<unsigned int>(m_noNodes - 1));
		for (i = 0; i < NUMOFANTS; i++) 
		{
			m_nnList[node][i] = *(pHelpArray+i);
		}
	}
	delete [] pHelpArray;
	delete [] pdistanceArray;
}

void CBestWorstAntSystem::initPheromones() 
{
	//initPheromoneTrails(trail_0);
	CAntSystem::initPheromones();
}

void CBestWorstAntSystem::initPheromoneTrails(double initialValue) const
{
	std::string s =format("init pheromone trails %.15f  @ %d",initialValue ,m_iterations);
	static string c;
	c=s;
	AfxGetMainWnd()->PostMessage(IDC_VERBOSE ,0, (LPARAM)&c);
	
	
	unsigned int i, j;
	for (i = 0; i < m_noNodes; i++)
	{
		for (j = 0; j <= i; j++) 
		{
			m_newPheromoneMatrix->set(i , j , initialValue);
		}
	}

	for(i = 0; i < m_noNodes; i++)
		m_newPheromoneMatrix->set(i , i , 0);
}


//The pheromone update is a little bit different: on each iteration, the best to date ant deposits an additional quantity of pheromone on paths it traveled:
void CBestWorstAntSystem::updatePheromones()
{
	//get the best at so far
	this->evaporateAllPhero();


	//global_update_pheromone( best_so_far_ant );
	double d_tau = (double)1/ m_bestSoFarPathlength; 
	for(size_t city = 1; city <m_bestSoFarPath.size(); city++)
	{
		size_t from = m_bestSoFarPath[city-1];
		size_t to = m_bestSoFarPath[city];
		this->m_newPheromoneMatrix->add(from,to , d_tau);
	}
//	worstIndex = this->computeAntsItterationWorstIndex();
	//bwas_worst_ant_update
	const CAnt &BestAnt = GetBestAnt();
	const CAnt &WorstAnt = getWorstAnt();

	bwas_worst_ant_update( WorstAnt, BestAnt );
	long distance_best_worst = distance_between_ants( WorstAnt ,BestAnt);
	if ( distance_best_worst < (int) (0.05 * (double) m_noNodes) ) 
	{
		m_RestartBestAnt.setAntTourLength((std::numeric_limits<int>::max)());
		initPheromoneTrails( this->m_newPheromoneMatrix->averagePheromoneTrail(m_bestSoFarPath));
		//CAntSystem::initPheromones();
		restart_iteration = m_iterations;
		restart_time = GetTickCount();
	}
	else 
	{
		bwas_pheromone_mutation();
	}
	calculateHeuristicMatrix();
 }
void CBestWorstAntSystem::bwas_pheromone_mutation( void )
{

	unsigned int     i, j, k;
	unsigned int     num_mutations;
	double       avg_trail = 0.0,  mutation_rate = 0.3;

     /* compute average pheromone trail on edges of global best solution */
	avg_trail = this->m_newPheromoneMatrix->averagePheromoneTrail(m_bestSoFarPath);

	//mutation_rate = mutation_rate / this->m_noNodes * this->m_noAnts;

    /* determine mutation strength of pheromone matrix */ 
    /* FIXME: we add a small value to the denominator to avoid any
       potential division by zero. This may not be fully correct
       according to the original BWAS. */
    //if ( max_time > 0.1 )

	//wrong here slowly gets with each irreatio
	
	double mutation_strength = 4.0 * (double)avg_trail * (double) (GetTickCount() - restart_time) / (30000- (restart_time + 0.0001));
	

	mutation_rate = mutation_rate / this->m_noNodes *m_noAnts;
	num_mutations = (int) (this->m_noNodes * mutation_rate) / 2;
		
	//num_mutations = static_cast<long>(m_noNodes * mutation_rate / 2);   

    /* / 2 because of adjustment for symmetry of pheromone trails */
 
    if ( restart_iteration < 2 )
	     num_mutations = 0; 

    for ( i = 0 ; i < num_mutations ; i++ ) 
	{
		j =   (int) (fRand(0, (unsigned int)m_newPheromoneMatrix->size()-1  ));
		k =   (int) (fRand(0, (unsigned int)m_newPheromoneMatrix->size()-1  ));
		double chance =  fRand(0,1);
		if ( chance < 0.5 ) 
		{
			this->m_newPheromoneMatrix->add(j,k , mutation_strength);
		}
		else 
		{	
			this->m_newPheromoneMatrix->subtract(j,k , mutation_strength);
			if ( this->m_newPheromoneMatrix->get(j,k) <= 0.0 ) 
			{
				this->m_newPheromoneMatrix->set1(j,k ,0.00000000000000000000000000000001 );
			}
			this->m_newPheromoneMatrix->set1(j,k , this->m_newPheromoneMatrix->get(j,k) );
		}
    }
}




CBestWorstAntSystem::~CBestWorstAntSystem(void)
{
}