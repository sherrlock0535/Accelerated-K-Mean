#include <cmath>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <vector>
using namespace std;    
class KMeans{
    class Point;
    class Data_point;
    class Center;
public:
    vector<int> result;
    int iterator_times;
    KMeans(vector<vector<double>> dataset, vector<int> label, int cluster_number){
        data_size = dataset.size();
        dimension = dataset[0].size();
        cluster = cluster_number;
        for (auto it = dataset.begin(); it != dataset.end();it++){
            vector<double> pos;
            for (int i = 0; i < dimension;i++)
                pos.push_back((*it)[i]);
            point.push_back(Data_point(pos));
        }
        for (auto it = label.begin(); it != label.end();it++)
            this->label.push_back(*it);
        //output(); 
        iterator_times = 0;
        mainThread();
       
    }
private:
    int data_size, dimension, cluster;
    vector<Data_point> point;
    vector<Center> center;
    vector<int> label;

    class Point{
    public:
        vector<double> pos;
        void build(vector<double> arr){
            pos.clear();
            for (auto it = arr.begin(); it != arr.end();it++)
                pos.push_back(*it);
        }
        void build(Point &rhs){
            pos.clear();
            for (auto it = rhs.pos.begin(); it != rhs.pos.end();it++)
                pos.push_back(*it);
        }
        void output(){
            cout << "point data:";
            for (auto it = pos.begin(); it != pos.end();it++)
                cout << (*it) << " ";
            cout << endl;
        }
    private:
        void clear(){
            pos.clear();
        }
    };

    class Data_point : public Point{
    public:
        int center;
        vector<double> lowerBound;
        double upperBound;
        bool uOutofDate, transed;
        Data_point(vector<double> arr){
            this->build(arr);
            center = 0;
            uOutofDate = transed = false;
        }
    };

    class Center : public Point{
    public:
        vector<double> mean;
        int cnt;
        vector<double> disc;
        double s;//s(c)=1/2 min(c,c')(c!=c')
        Center(Data_point &rhs){
            this->build(rhs);
            cnt = 0;
        }

        void trans(){
            int len = mean.size();
            for (int i = 0; i < len;i++){
                pos[i]=mean[i];
                mean[i] = 0;
            }
            cnt = 0;
        }

        void add_mean(Data_point &data){
            int len = mean.size();
            for (int i = 0; i < len;i++)
                mean[i] += data.pos[i];
            cnt++;
        }

        void cal_mean(){
            int len = mean.size();
            for (int i = 0; i < len;i++)
                mean[i] /= cnt;
        }

        void output(){
            int len = pos.size();
            cout << "center position:";
            for (int i = 0; i < pos.size();i++)
                cout << pos[i] << " ";
            cout << endl;
        }
    };

    double cal_dis(Point &a, Point &b){
        double dis = 0;
        for (int i = 1; i <= this->dimension; i++){
            dis += (a.pos[i] - b.pos[i]) * (a.pos[i] - b.pos[i]);
        }
        return sqrt(dis);
    }
    
    double cal_dis(Point &a, vector<double> &b){
        double dis = 0;
        for (int i = 1; i <= this->dimension; i++){
            dis += (a.pos[i] - b[i]) * (a.pos[i] - b[i]);
        }
        return sqrt(dis);
    }

    void output(){
        cout << "Data size: " << data_size << endl;
        cout << "Dimension: " << dimension << endl;
        cout << "Data:" << endl;
        // for (auto it = point.begin(); it != point.end();it++)
        //     it->output();
    }

    //Calculate distance between centers and s(c)
    void cal_disc(){
        for (int c1 = 0; c1 < cluster;c1++)
            for (int c2 = c1+1; c2 < cluster;c2++){
                double dis = cal_dis(center[c1], center[c2]);
                center[c1].disc[c2] = dis;
                center[c2].disc[c1] = dis;
            }
        for (int c1 = 0; c1 < cluster;c1++){
            double s = center[c1].disc[(c1 + 1) % cluster];
            for (int c2 = 0; c2 < cluster;c2++){
                if(c1==c2)
                    continue;
                double s2 = center[c1].disc[c2];
                if(s2<s)
                    s = s2;
            }
            center[c1].s = s / 2;
        }
    }

    void init(){
        //get initial centers
        vector<int> num;
        for (int i = 0; i < data_size;i++)
            num.push_back(i);
        srand(time(NULL));
        for (int i = 0; i < data_size; i++)
            swap(num[i],num[rand() % data_size]);
        for (int i = 0; i < cluster; i++)
            center.push_back(Center(point[num[i]]));
        num.clear();

        //Initial the center
        for (auto c = center.begin(); c != center.end();c++){
            for (int i = 0; i < cluster;i++)
                c->disc.push_back(0);
            for (int i = 0; i < dimension;i++)
                c->mean.push_back(0);
        }
        cal_disc();
        //Calculate initial c(x) and l(x,c) and u(x)
        for (auto x = point.begin(); x != point.end();x++){
            for (int i = 0; i < cluster;i++)
                x->lowerBound.push_back(0);
            double dis = cal_dis(*x, center[0]);
            x->center = 0;
            int len = center.size();
            for (int c = 1; c < len;c++){
                if(dis<=center[x->center].disc[c])
                    continue;
                double dis2 = cal_dis(*x, center[c]);
                x->lowerBound[c] = dis2;
                if(dis2<dis){
                    x->center = c;
                    dis = dis2;
                }
            }
            x->upperBound = dis;
            center[x->center].add_mean(*x);
        }
        for (auto c = center.begin(); c != center.end();c++)
            c->cal_mean();

        //Move the centers
        // cout << "first time center:" << endl;
        // for (auto c = center.begin(); c != center.end();c++)
        //     c->output();

        for (auto c = center.begin(); c != center.end();c++)
            c->trans();

        // cout << "second time center:" << endl;
        // for (auto c = center.begin(); c != center.end();c++)
        //     c->output();
    }   

    //Repeat steps
    bool repeat(){
        bool flag = false;
        cal_disc();
        for (auto x = point.begin(); x != point.end();x++){
            x->transed = false;
            double disc;
            //Ignore points which will not trans
            if(x->upperBound<=center[x->center].s)
                continue;
            //Update the upperBound
            if(x->uOutofDate){
                x->uOutofDate=false;
                disc = cal_dis(*x, center[x->center]);
                x->upperBound = disc;
            }else
                disc = x->upperBound;
            //trans points to closer centers
            for (int c = 0; c < cluster;c++){
                if(disc>x->lowerBound[x->center]||disc>0.5*center[x->center].disc[c]){
                    double disc2 = cal_dis(*x, center[c]);
                    //cout << "iter note:"<<disc<<" "<<disc2 << endl;
                    if(disc2<disc){
                        //cout << "yes" << endl;
                        //cout << "trans note:" << disc2 << " " << disc << endl;
                        x->center = c;
                        x->transed = true;
                        flag = true;
                    }
                }
            }
            //Add to the center
            center[x->center].add_mean(*x);
        }
        //Calculate the mean of centers
        for (auto c = center.begin(); c!=center.end();c++)
            c->cal_mean();
        
        //Update the lowerBound and upperBound
        for (auto x = point.begin(); x != point.end();x++){
            if(!x->transed)
                continue;
            for (int c = 0; c < cluster;c++)
                x->lowerBound[c] = max(x->lowerBound[c] - cal_dis(center[c], center[c].mean), 0.0);
            x->upperBound += cal_dis(center[x->center], center[x->center].mean);
            x->uOutofDate = true;
        }

        //Move the centers
        for (auto c = center.begin(); c != center.end();c++)
            c->trans();

        iterator_times++;
        return flag;
    }
    //Get result and save to `result`
    void getResult(){
        for (auto x = point.begin(); x != point.end();x++)
            result.push_back((*x).center);
    }

    void mainThread(){
        init();
        while(true){
            if(!repeat())
                break;
        }
        getResult();
    }   
    
};

int main(){
    // Get the data
    freopen("seeds_dataset.txt", "r", stdin);
    vector<vector<double>> dataset;
    vector<int> label;
    double tmp;
    int tmp2;
    for (int i = 1; i <= 210; i++){
        vector<double> point;
        for (int j = 1; j <= 7; j++){
            cin >> tmp;
            point.push_back(tmp);
        }
        cin >> tmp2;
        label.push_back(tmp2);
        dataset.push_back(point);
    }
    //KMeans(vector<vector<double>> dataset, vector<int> label, int number of cluster)
    KMeans model(dataset, label, 3);

    cout << "result:" << endl;
    for (auto it = model.result.begin(); it != model.result.end();it++)
        cout << (*it) << " ";
    cout << endl;

    cout << "iteration times: " << model.iterator_times << endl;
}