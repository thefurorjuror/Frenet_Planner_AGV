#include "../include/frenet_optimal_trajectory.hpp"
#include <ros/console.h>
#include <bits/stdc++.h>
#include <algorithm>
#include <vector>
namespace plt = matplotlibcpp;
int transform_count=0;

// calculates lateral paths using the sampling parameters passed
void FrenetPath::calc_lat_paths (double c_d, double c_d_d, double c_d_dd, double Ti, double di,
double di_d)
{
  // trace(c_d,c_d_d,c_d_dd,Ti,di,di_d);
  int n=1+Ti/DT;
	t.resize(n);
	d.resize(n);
	d_d.resize(n);
	d_dd.resize(n);
	d_ddd.resize(n);
	// omp_set_num_threads(4);
	//double startTime = omp_get_wtime();
	// #pragma omp for simd 
	// {
		
  quintic lat_qp(c_d, c_d_d, c_d_dd, di, di_d, 0.0, Ti);
  for(int te = 0; te <n; te ++)
		{
			t[te] =te*DT;
			d[te]= lat_qp.calc_point(te*DT);
			d_d[te] = lat_qp.calc_first_derivative(te*DT);
			d_dd[te]= lat_qp.calc_second_derivative(te*DT);
			d_ddd[te] = lat_qp.calc_third_derivative(te*DT);
		}
}

// calculates longitudnal paths  using quartic polynomial
void FrenetPath::calc_lon_paths(double c_speed, double s0, double Ti, double tv)
{
  // Trying to give an initial acceleration in the sampling
  /*if (c_speed< 0.01)
  {
    ROS_INFO("LOW SPEED");
    quartic lon_qp(s0, c_speed, 10.0, tv, 0.0, Ti);	//s_dd (longitudnal accln.)is set constant for low speed
    for(auto const& te : t) 
    {
      s.push_back(lon_qp.calc_point(te));
      s_d.push_back(lon_qp.calc_first_derivative(te));
      s_dd.push_back(lon_qp.calc_second_derivative(te));
      s_ddd.push_back(lon_qp.calc_third_derivative(te));
    }
  }
  else
  {
    quartic lon_qp(s0, c_speed, 0.0, tv, 0.0, Ti);	//s_dd is set to const. 0 (i.e. not being sampled) 
    for(auto const& te : t) 
    {
      s.push_back(lon_qp.calc_point(te));
      s_d.push_back(lon_qp.calc_first_derivative(te));
      s_dd.push_back(lon_qp.calc_second_derivative(te));
      s_ddd.push_back(lon_qp.calc_third_derivative(te));
    }
  }*/

  // Normal Implementation
  quartic lon_qp(s0, c_speed, 0.0, tv, 0.0, Ti);  // s_dd is set to const. 0 (i.e. not being
                                                  // sampled)
  int size = t.size();
  s.resize(size);
  s_d.resize(size);
  s_dd.resize(size);
  s_ddd.resize(size);
  int i = 0;
  for(auto const te : t)
  {
    s[i] = (lon_qp.calc_point(te));
    s_d[i] = (lon_qp.calc_first_derivative(te));
    s_dd[i] = (lon_qp.calc_second_derivative(te));
    s_ddd[i] = (lon_qp.calc_third_derivative(te));
    ++i;
  }

  // https://www.geeksforgeeks.org/std-inner_product-in-cpp/
  Js = inner_product(s_ddd.begin(), s_ddd.end(), s_ddd.begin(), 0);
  double ds = pow((TARGET_SPEED - s_d.back()), 2);
  cd = (KJ*Jp + KT*Ti + KD*d.back()*d.back());
  cv = (KJ*Js + KT*Ti + KD_V*ds);
  cf = (KLAT*cd + KLON*cv);
}

// calculates longitudnal paths  using quintic polynomial
void FrenetPath::calc_lon_paths_quintic_poly(double c_speed, double s0, double Ti, double ts,
double tv)
{
  /*if(abs(s_dest-s0) <= 16)
  {
    quintic lon_qp(s0, c_speed, 0.0, min(a1,a2);, tv, 0.0, Ti);  // s_dd is not being sampled
    int size = t.size();
    s.resize(size);
    s_d.resize(size);
    s_dd.resize(size);
    s_ddd.resize(size);
    int i = 0;
    for(auto te : t)
    {
      s[i] = (lon_qp.calc_point(te));
      s_d[i] = (lon_qp.calc_first_derivative(te));
      s_dd[i] = (lon_qp.calc_second_derivative(te));
      s_ddd[i] = (lon_qp.calc_third_derivative(te));
      ++i;
    }
    // https://www.geeksforgeeks.org/std-inner_product-in-cpp/
    Js = inner_product(s_ddd.begin(), s_ddd.end(), s_ddd.begin(), 0);

    double ds = pow((TARGET_SPEED - s_d.back()), 2);
    /*if(STOP_CAR && s_d.size() >= 2)
      ds = s_d[1]*s_d[1];
    // calculation of lateral, longitudnal and overall cost of the trajectories
    cd = (KJ*Jp + KT*Ti + KD*d.back()*d.back());
    cv = (KJ*Js + KT*Ti + KD_V*ds);
    cf = (KLAT*cd + KLON*cv);
  } else {
    quintic lon_qp(s0, c_speed, 0.0, s0 +15, tv, 0.0, Ti);  // s_dd is not being sampled
    int size = t.size();
    s.resize(size);
    s_d.resize(size);
    s_dd.resize(size);
    s_ddd.resize(size);
    int i = 0;
    for(auto te : t)
    {
      s[i] = (lon_qp.calc_point(te));
      s_d[i] = (lon_qp.calc_first_derivative(te));
      s_dd[i] = (lon_qp.calc_second_derivative(te));
      s_ddd[i] = (lon_qp.calc_third_derivative(te));
      ++i;
    }
    // https://www.geeksforgeeks.org/std-inner_product-in-cpp/
    Js = inner_product(s_ddd.begin(), s_ddd.end(), s_ddd.begin(), 0);

    double ds = pow((TARGET_SPEED - s_d.back()), 2);
    /*if(STOP_CAR && s_d.size() >= 2)
      ds = s_d[1]*s_d[1];
    // calculation of lateral, longitudnal and overall cost of the trajectories
    cd = (KJ*Jp + KT*Ti + KD*d.back()*d.back());
    cv = (KJ*Js + KT*Ti + KD_V*ds);
    cf = (KLAT*cd + KLON*cv);

  }*/
  quintic lon_qp(s0, c_speed, 0.0, min(s0+15,203.5), tv, 0.0, Ti);  // s_dd is not being sampled
  int size = t.size();
  s.resize(size);
  s_d.resize(size);
  s_dd.resize(size);
  s_ddd.resize(size);
  int i = 0;
  for(auto te : t)
  {
    s[i] = (lon_qp.calc_point(te));
    s_d[i] = (lon_qp.calc_first_derivative(te));
    s_dd[i] = (lon_qp.calc_second_derivative(te));
    s_ddd[i] = (lon_qp.calc_third_derivative(te));
    ++i;
  }
  if(STOP_CAR){
    for(int i=0;i<s.size();i++)
    {
        if(s[i]>203.5)
          {
            s.resize(i);
            s_d.resize(i);
            s_dd.resize(i);
            s_ddd.resize(i);
            t.resize(i);
            d.resize(i);
            d_d.resize(i);
            d_dd.resize(i);
            d_ddd.resize(i);
            vecD d_ddd_vec = d_ddd;
            Jp = ( inner_product(d_ddd_vec.begin(), d_ddd_vec.end(), d_ddd_vec.begin(), 0));
            break;
          }
    }

  }
  

  // https://www.geeksforgeeks.org/std-inner_product-in-cpp/
  Js = inner_product(s_ddd.begin(), s_ddd.end(), s_ddd.begin(), 0);

  double ds = pow((TARGET_SPEED - s_d.back()), 2);
  if(STOP_CAR && s_d.size() >= 2)
    ds = s_d[1]*s_d[1];
  // calculation of lateral, longitudnal and overall cost of the trajectories
  cd = (KJ*Jp + KT*Ti + KD*d.back()*d.back());
  cv = (KJ*Js + KT*Ti + KD_V*ds);
  cf = (KLAT*cd + KLON*cv);
}

// get sampling limits of d using the previously calculated paths (if present)
void get_limits_d(FrenetPath lp, double *lower_limit_d, double *upper_limit_d)
{
  vecD d_sampling = lp.get_d();
  if(d_sampling.size() != 0)
  {
    *lower_limit_d = d_sampling.back() - MAX_SHIFT_D;
    *upper_limit_d = d_sampling.back() + MAX_SHIFT_D + D_ROAD_W;
  }
}

// generates frenet path parameters
vector<FrenetPath> calc_frenet_paths(double c_speed, double c_d, double c_d_d, double c_d_dd,
double s0, FrenetPath lp)
{
  // trace(c_d_d);
  vector<FrenetPath> frenet_paths;
  double lower_limit_d, upper_limit_d;
  lower_limit_d = -MAX_ROAD_WIDTH;
  upper_limit_d = MAX_ROAD_WIDTH + D_ROAD_W;
  get_limits_d(lp, &lower_limit_d, &upper_limit_d);  // IF not required to sample around previous
                                                     // sampled d(th) then comment this line. 
  if(STOP_CAR)
  {
    #pragma omp parallel for collapse(2)
   for(int Di = int(lower_limit_d/ D_ROAD_W); Di <= int(upper_limit_d/ D_ROAD_W); Di += 1)  // sampling for lateral
                                                                          // offset
    {
      for(int Ti = int(MINT/DT); Ti <= int((MAXT + DT)/DT); Ti += 1)  // Sampling for prediction time
      {
        double di = double(Di)/D_ROAD_W;
        double ti = double(Ti)/D_T_S;
        FrenetPath fp;
        FrenetPath tfp;
        fp.calc_lat_paths(c_d, c_d_d, c_d_dd, ti, di, TARGET_SPEED);
        vecD d_ddd_vec = fp.get_d_ddd();
        fp.set_Jp( inner_product(d_ddd_vec.begin(), d_ddd_vec.end(), d_ddd_vec.begin(), 0));
        tfp = fp;
        tfp.calc_lon_paths_quintic_poly(c_speed, s0, ti, 15, TARGET_SPEED);
        #pragma omp critical
        frenet_paths.push_back(tfp);
      }
    }
  } else {

    #pragma omp parallel for collapse(3)
    for(int Di = int(lower_limit_d/ D_ROAD_W); Di <= int(upper_limit_d/ D_ROAD_W); Di += 1)  // sampling for lateral
                                                                          // offset
    {
      for(int Ti = int(MINT/DT); Ti <= int((MAXT + DT)/DT); Ti += 1)  // Sampling for prediction time
      {
          
        for(int Di_d = int(-MAX_LAT_VEL/D_D_NS); Di_d <= int((MAX_LAT_VEL + D_D_NS)/D_D_NS); Di_d+=1)
        {
          double di = double(Di)*D_ROAD_W;
          double ti = double(Ti)*DT;
          double di_d = double(Di_d)*D_D_NS;
          
          FrenetPath fp;
          FrenetPath tfp;
          fp.calc_lat_paths(c_d, c_d_d, c_d_dd, ti, di, di_d);
          vecD d_ddd_vec = fp.get_d_ddd();
          fp.set_Jp( inner_product(d_ddd_vec.begin(), d_ddd_vec.end(), d_ddd_vec.begin(), 0));
          double minV = TARGET_SPEED - D_T_S*N_S_SAMPLE;
          double maxV = TARGET_SPEED + D_T_S*N_S_SAMPLE;

          // sampling for longitudnal velocity
          for(double tv = minV; tv <= maxV + D_T_S; tv += D_T_S)
          {
            tfp = fp;
            tfp.calc_lon_paths_quintic_poly(c_speed, s0, ti, 15, tv);
            #pragma omp critical
            frenet_paths.push_back(tfp);
          }
        }
      }
    }

  }
  /*
  for(double di = lower_limit_d; di <= upper_limit_d; di += D_ROAD_W)  // sampling for lateral
                                                                       // offset
  {
    for(double Ti = MINT; Ti <= MAXT + DT; Ti += DT)  // Sampling for prediction time
    {
      if(STOP_CAR)
      {
          FrenetPath fp;
          FrenetPath tfp;
          fp.calc_lat_paths(c_d, c_d_d, c_d_dd, Ti, di, TARGET_SPEED);
          vecD d_ddd_vec = fp.get_d_ddd();
          fp.set_Jp( inner_product(d_ddd_vec.begin(), d_ddd_vec.end(), d_ddd_vec.begin(), 0));
          tfp = fp;
          tfp.calc_lon_paths_quintic_poly(c_speed, s0, Ti, 15, TARGET_SPEED);
          frenet_paths.push_back(tfp);
      }
      else {
        // Sampling for lateral velocity
        for(double di_d = -MAX_LAT_VEL; di_d <= MAX_LAT_VEL + D_D_NS; di_d+=D_D_NS)
        {
          FrenetPath fp;
          FrenetPath tfp;
          fp.calc_lat_paths(c_d, c_d_d, c_d_dd, Ti, di, di_d);
          vecD d_ddd_vec = fp.get_d_ddd();
          fp.set_Jp( inner_product(d_ddd_vec.begin(), d_ddd_vec.end(), d_ddd_vec.begin(), 0));
          double minV = TARGET_SPEED - D_T_S*N_S_SAMPLE;
          double maxV = TARGET_SPEED + D_T_S*N_S_SAMPLE;

          // sampling for longitudnal velocity
          for(double tv = minV; tv <= maxV + D_T_S; tv += D_T_S)
          {
            tfp = fp;
            tfp.calc_lon_paths_quintic_poly(c_speed, s0, Ti, 15, tv);
            frenet_paths.push_back(tfp);
          }
        }
      }
    }
  }
  */
  return frenet_paths;
}

void FrenetPath::adding_global_path(Spline2D csp)
{
  double startTime1 = omp_get_wtime();
  int n = s.size();
  //cerr<<"n is "<<n<<endl;
  /*if(STOP_CAR)
  {
    cerr<<"s_dest ="<<s_dest<<" size is "<<n<<" is"<<endl;
    for(auto i : s)
      cerr<<i<<" ";
    cerr<<endl;
  }*/
  x.resize(n);
  y.resize(n);
  for(unsigned int i = 0; i < n; i++)
  {
    double ix, iy;
    // trace(i);
    if(STOP_CAR){
      //cerr<<"310"<<endl;
    }
    
    csp.calc_position(ix, iy, s[i]);
    // trace("done calc_position");
    if(STOP_CAR){
      //cerr<<"316 "<<ix<<" "<<iy<<endl;
    }
    if(ix == NONE)
    {
      // if(STOP_CAR)
      //   cerr<<"hii"<<endl;
      return;
      //break;
    }
    if(STOP_CAR){
      //cerr<<"324"<<endl;
    }
    double iyaw = csp.calc_yaw(s[i]);
    if(STOP_CAR){
      //cerr<<"328 "<<iyaw<<endl;
    }
    //double di = d[i];
    double fx = ix - d[i]*sin(iyaw);
    double fy = iy + d[i]*cos(iyaw);
    x[i] = (fx);
    y[i] = (fy);
  }
  if(STOP_CAR){
    cerr<<"\n\nHOlaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\n\n";
  }
  yaw.resize(n-1);
  ds.resize(n-1);
  
  for(unsigned int i = 0; i < n - 1; i++)
  {
    double dx = x[i + 1] - x[i];
    double dy = y[i + 1] - y[i];
    if(abs(dx)>0.0001){
        yaw[i] = (atan2(dy, dx));
    }
    else{
      yaw[i]=0;
    }
    
    ds[i] = (sqrt(dx*dx + dy*dy));
  }
  // TO remove paths whose predicted s goes out of bounds of global path.
  if(s.size() == x.size())
  {
    //if(STOP_CAR)
      //cerr<<"hii"<<endl;
    return;
  }
  c.resize((n-1) - 1);
  for(unsigned int i = 0; i < (n-1) - 1; i++){
    if(ds[i]!=0)
     c[i]=((yaw[i + 1] - yaw[i]) / ds[i]);
    else
    {
      c[i]=0;
    }
    
  }

  double endTime1 = omp_get_wtime();
  /*x.clear();
  y.clear();
  yaw.clear();
  ds.clear();
  c.clear();*/

  /*double startTime2 = omp_get_wtime();

//s.size() ko n set karde
  for(unsigned int i = 0; i < s.size(); i++)
  {
    double ix, iy;
    csp.calc_position(ix, iy, s[i]);
    if(ix == NONE) // agar yeh condition true hoga toh x.size and s.size equal nhi honge toh yehi return mardete hai
    {
      break;
    }
    double iyaw = csp.calc_yaw(s[i]);
    double di = d[i]; //dont use this 
    double fx = ix - di*sin(iyaw);
    double fy = iy + di*cos(iyaw);
    x.push_back(fx); // remove pushback
    y.push_back(fy); // remove pushback
  }
//x.size() ko const kardo
  for(unsigned int i = 0; i < x.size() - 1; i++)
  {
    double dx = x[i + 1] - x[i]; //remove these two variables and parallelise???
    double dy = y[i + 1] - y[i];
    yaw.push_back(atan2(dy, dx)); //remove pushback
    ds.push_back(sqrt(dx*dx + dy*dy)); // remove pushback
  }
  // TO remove paths whose predicted s goes out of bounds of global path.
  if(s.size() - x.size() != 0) //(s.size()==x.size()) should be faster
  {
    return;
  }
//yaw.size ko bhi constant kardo
  for(unsigned int i = 0; i < yaw.size() - 1; i++){
    c.push_back((yaw[i + 1] - yaw[i]) / ds[i]); //pushback hata de 
  }

  double endTime2 = omp_get_wtime();*/

  

  // cerr<<"Global Path Time 1:"<<endTime1-startTime1<<endl;
  // cerr<<"Global Path Time 2:"<<endTime2-startTime2<<endl;
}

// convert the frenet paths to global frame
vector<FrenetPath> calc_global_paths(vector<FrenetPath> fplist, Spline2D csp)
{
  int n=fplist.size();
  #pragma omp parallel for collapse(1)
  for(int i=0;i<n;i++)
  {
    //FrenetPath fp = fplist[i];
    fplist[i].adding_global_path(csp);
  }
  return fplist;
}

// transforms robot's footprint
vector<geometry_msgs::Point32> transformation(vector<geometry_msgs::Point32> fp,
geometry_msgs::Pose cp, double px, double py, double pyaw)
{
  //transform_count++;
  vector<geometry_msgs::Point32> new_fp(fp.size());
  tf::Quaternion qb(cp.orientation.x, cp.orientation.y, cp.orientation.z, cp.orientation.w);
  tf::Matrix3x3 mb(qb);
  double broll, bpitch, byaw;
  mb.getRPY(broll, bpitch, byaw);
  double bx, by;
  bx = cp.position.x;
  by = cp.position.y;
  double x, y, theta;
  theta = pyaw - byaw;
  x = px - bx;
  y = py - by;
  int n = new_fp.size();
  double startTime2 = omp_get_wtime();
  for(int i = 0; i < n; i++)
  {
    new_fp[i].x = (fp[i].x - bx)* cos(theta) + (fp[i].y - by) * sin(theta) + x + bx;
    new_fp[i].y = -(fp[i].x - bx) * sin(theta) + (fp[i].y - by) * cos(theta) + y + by;
  }
  double endTime2 = omp_get_wtime();
  //cerr<<"Transform Time 2:"<<endTime2-startTime2<<endl;
  return new_fp;
}

// returns distance between two points
#pragma omp declare simd
double dist(double x1, double y1, double x2, double y2)
{
  return sqrt((x1 - x2)*(x1 - x2) + (y1 - y2)*(y1 - y2));
}

bool point_obcheck(geometry_msgs::Point32 p, double obst_r)
{
  int xlower, ylower, xupper, yupper;
  auto it = lower_bound(ob_x.begin(), ob_x.end(), p.x);
  if (ob_x.size() == 0)
  {
    return 0;
  }
  if (it == ob_x.begin())
  {
    xlower = xupper = it - ob_x.begin();  // no smaller value than val in vector
  } else if (it == ob_x.end()) {
    xupper = xlower = (it-1)- ob_x.begin();  // no bigger value than val in vector
  } else {
      xlower = (it-1) - ob_x.begin();
      xupper = it - ob_x.begin();
  }
  double dist1 = dist(p.x, p.y, ob_x[xlower], ob_y[xlower]);
  double dist2 = dist(p.x, p.y, ob_x[xupper], ob_y[xupper]);
  if(min(dist1, dist2) < obst_r)
  {
    return 1;
  }
  it = lower_bound(ob_y.begin(), ob_y.end(), p.y);
  if (it == ob_y.begin())
  {
    ylower = yupper = it - ob_y.begin();  // no smaller value  than val in vector
  } else if (it == ob_y.end())  {
    yupper = ylower = (it-1)- ob_y.begin();  // no bigger value than val in vector
  } else {
      ylower = (it-1) - ob_y.begin();
      yupper = it - ob_y.begin();
  }
  dist1 = dist(p.x, p.y, ob_x[ylower], ob_y[ylower]);
  dist2 = dist(p.x, p.y, ob_x[yupper], ob_y[yupper]);
  if(min(dist1, dist2) < obst_r)
  {
    return 1;
  }
  return 0;
}

// checks for collision of the bot
bool FrenetPath::check_collision(double obst_r)
{
  // trace("in");
  if(s.size() != x.size())
  {
    // trace("307");
    return 1;
  }
  for (unsigned int i = 0; i < min(x.size(), yaw.size()); i++)
  {
    // trace("poss");
    vector<geometry_msgs::Point32> trans_footprint = transformation(footprint.polygon.points,
    odom.pose.pose, x[i], y[i], yaw[i]);
    // trace("tno");
    for(unsigned int j = 0; j < trans_footprint.size(); j++)
    {
      // trace(i,j);
      if (point_obcheck(trans_footprint[j], obst_r) ==1 )
      {
        // trace("out1");
        return 1;
      }
    }
  }
  // trace("out2");
  return 0;
}

inline bool sortByCost(FrenetPath a, FrenetPath b)
{
  if (a.get_cf() != b.get_cf()){
    return a.get_cf() < b.get_cf();
  } else {
    double jerkCost1, jerkCost2;
    jerkCost1 = KLAT * a.get_Jp() + KLON * a.get_Js();
    jerkCost2 = KLAT * b.get_Jp() + KLON * b.get_Js();
    return jerkCost1 < jerkCost2;
  }
}

// check for specified velocity, acceleration, curvature constraints and collisions
vector<FrenetPath> check_path(vector<FrenetPath>& fplist, double bot_yaw, double yaw_error,
double obst_r)
{
  vector<FrenetPath> fplist_final;
 
  #pragma omp parallel for collapse(1)
  for(unsigned int i = 0; i < fplist.size(); i++)
  {
     FrenetPath fp = fplist[i];
    int flag = 0;
    vecD path_yaw = fplist[i].get_yaw();
    if (path_yaw.size() == 0)
      continue;
    if ((path_yaw[0] - bot_yaw)> yaw_error || (path_yaw[0] - bot_yaw)< -yaw_error)  // 20 deg
    {
      flag = 1;
    }
    if(flag == 1){continue;}
    else if(fp.check_collision(obst_r) == 0)
    {
      #pragma omp critical
      fplist_final.push_back(fplist[i]);
    }
  }
  return fplist_final;
}

void FrenetPath::plot_path()
{
  plt::plot(x, y);
  plt::pause(0.001);
}
void FrenetPath::plot_velocity_profile()
{
  plt::plot(t, s_d);
  plt::pause(0.001);
}

static int flag_for_display_paths = 0;

void display_paths(vector<FrenetPath> fplist)
{
  plt::ion();
  plt::show();
  int count = 0;
  for(auto &fp : fplist)
  {
    if(count%50 == 0 && flag_for_display_paths){
      fp.plot_path();
    }
    count++;
  }
  flag_for_display_paths = 1;
}

// generates the path and returns the bestpath
FrenetPath frenet_optimal_planning(Spline2D csp, double s0, double c_speed, double c_d,
double c_d_d, double c_d_dd, FrenetPath lp, double bot_yaw)
{
  trace("start");
  double startTime1 = omp_get_wtime();
  vector<FrenetPath> fplist = calc_frenet_paths(c_speed, c_d, c_d_d, c_d_dd, s0, lp);
  double endTime1 = omp_get_wtime();
  
  
  
  //cerr<<"sixe: "<<fplist.size()<<endl;
  trace("calc_global_paths");
  double startTime2 = omp_get_wtime();
  fplist = calc_global_paths(fplist, csp);
  double endTime2 = omp_get_wtime();
  trace("check_path");
  if(STOP_CAR){

    double min_cost = FLT_MAX;
    double cf;
    FrenetPath bestpath;
    for(auto & fp : fplist)
    {
      cf = fp.get_cf();
      if(min_cost >= cf)
      {
        min_cost = cf;
        bestpath = fp;
      }
    }
  //   cerr<<endl<<bestpath.get_d_d().size()<<endl;
  // cerr<<bestpath.get_s_d().size()<<endl;
      cerr<<endl<<" d size="<<bestpath.get_d().size()<<endl;
      for(auto i : bestpath.get_d())
        cerr<<i<<"  ";
      cerr<<endl<<" s size="<<bestpath.get_s().size()<<endl;
      for(auto i : bestpath.get_s())
        cerr<<i<<"  ";
      cerr<<endl<<" x size="<<bestpath.get_x().size()<<endl;
      for(auto i : bestpath.get_x())
        cerr<<i<<"  ";
      cerr<<endl<<" y"<<endl;
      for(auto i : bestpath.get_y())
        cerr<<i<<"  ";
      cerr<<endl<<" yaw"<<endl;
      for(auto i : bestpath.get_yaw())
        cerr<<i<<"  ";
      cerr<<endl<<" ds"<<endl;
      for(auto i : bestpath.get_ds())
        cerr<<i<<"  ";
      cerr<<endl<<" c"<<endl;
      for(auto i : bestpath.get_c())
        cerr<<i<<"  ";
  }
  
  transform_count = 0;
  // for now maximum possilble paths are taken into list
  double startTime3 = omp_get_wtime();
  fplist = check_path(fplist, bot_yaw, 0.523599, 2.0);
  double endTime3 = omp_get_wtime();
  //cerr<<"transform_count "<<transform_count<<endl;
  trace("done checking ");
  
  // cerr<<"Time 1:"<<endTime1-startTime1<<endl;
  // cerr<<"Time 2:"<<endTime2-startTime2<<endl;
  // cerr<<"Time 3:"<<endTime3-startTime3<<endl;
  // For displaying all paths
  if(false)
  {
    display_paths(fplist);
  }

  double min_cost = FLT_MAX;
  double cf;
  FrenetPath bestpath;
  for(auto & fp : fplist)
  {
    cf = fp.get_cf();
    if(min_cost >= cf)
    {
      min_cost = cf;
      bestpath = fp;
    }
  }
  // cerr<<bestpath.get_d_d().size()<<endl;
  // cerr<<bestpath.get_s_d().size()<<endl;
  // For showing the bestpath
  if(false)
  {
    plt::ion();
    plt::show();
    bestpath.plot_path();
  }
  // For plotting velocity profile (x,y) = (t,s_d)
  if(true)
  {
    plt::ion();
    plt::show();
    bestpath.plot_velocity_profile();
  }
  trace("DONE");
  if(STOP_CAR)
  {
    vector<double> chalja = bestpath.get_s_d();
    if(chalja.size() >= 2)
      cerr<< "BEST PATH S_D[1] = " << chalja[1] << endl;
  }
  return bestpath;
}
