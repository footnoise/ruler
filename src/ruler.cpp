
#include <map>
#include <iostream>
#include <iomanip>
#include <vector>
#include <string>

using std::vector;
using std::map;
using std::string;

namespace Ruler
{
	enum Token {NAME, VALUE, SPACE, ASSIGN='=', QUESTION='?', PRINT, END};
	
	typedef double MetricValue;
	
	/*
	    class stored <v1><u1>=<v2><u2> values 
	 */
	class Metric
	{
	private:
		string _name;
		/*
		  for example: 3600 seconds = 1 hour
		  map      <------------|
		  <metric> --> map      |
		  3600         <metric>-|
		               1
		 */
		map<Metric*, MetricValue> _values;
	public:
		Metric(string name);
		~Metric();
		void		Insert(Metric*, MetricValue);
		string		GetName();
		MetricValue	ConverTo(string, MetricValue value = 1, string ignore = "");
		map<Metric*, MetricValue>	GetValues();
	};
        /*
	    Store and covert values 
	 */
	class Ruler
	{
	private:
		vector<Metric*> _metrics;
		string _name;
		Ruler* _converter;

	public:
		Ruler(string);
		~Ruler();
		void Insert(Metric*);
		Metric* CreateMetric(string);
		Metric* Find(string);
		void Using(Ruler*);
		void ConvertAll();
		vector<Metric*> GetMetrics();
	};
	/*
	    Display results
	 */
	class Reporter
	{
	private:
		Ruler* _ruler;
		MetricValue _min;
		MetricValue _max;
	public:
		Reporter(Ruler*);
		~Reporter();
		void Set(MetricValue, MetricValue);
		void Show();
	};
	
	vector<Metric*> Ruler::GetMetrics()
	{
		return _metrics;
	}
	
	Reporter::Reporter(Ruler* ruler)
	{
		_ruler = ruler;
	}
	
	Reporter::~Reporter()
	{
	}
	
	void Reporter::Set(MetricValue min, MetricValue max)
	{
		_min = min;
		_max = max;
	}

	void Reporter::Show()
	{
		vector<Metric*> metrics = _ruler->GetMetrics();
		
		for (vector<Metric*>::iterator i = metrics.begin(); i != metrics.end(); i++)
		{
			map<Metric*, MetricValue> values = (*i)->GetValues();
			
			for (map<Metric*, MetricValue>::iterator j = values.begin(); j != values.end(); j++)
			{
				
				if (!((*j).first->GetValues())[(*i)])
				{
					std::cout << "No conversion is possible." << std::endl;
				}
				else
				{
					if (((*j).second < _min) || (*j).second > _max)
					{
						std::cout << std::scientific;						
					}
					else
					{
						std::cout << std::fixed;
					}
					std::cout << (*j).second << " " << ((*j).first)->GetName() << " = ";
					
					try
					{ 
						((*j).first->GetValues()[(*i)]);

					}
					catch (...)
					{
							
					}

					if ((((*j).first->GetValues())[(*i)] < _min) || (((*j).first->GetValues())[(*i)] > _max))
					{
						std::cout << std::scientific;						
					}
					else
					{
						std::cout << std::fixed;
					}
					
					std::cout << ((*j).first->GetValues())[(*i)] << " " << (*i)->GetName() <<  std::endl;
				}
			}
		}
	}
	
	Ruler::Ruler(string name):
	_name(name)
	{
	}
	
	Ruler::~Ruler()
	{
	    for (vector<Metric*>::iterator i = _metrics.begin(); i < _metrics.end(); i++)
	    {
		delete (*i);
	    }

	}
	
	void Ruler::Insert(Metric* m)
	{
		_metrics.push_back(m);
	}

	Metric* Ruler::Find(string name)
	{
		for (vector<Metric*>::iterator i = _metrics.begin(); i != _metrics.end(); i++)
		{
			if ((*i)->GetName() == name)
			{
				return (*i);
			}
		}

		return 0;
	}
	
	void Ruler::Using(Ruler* ruler)
	{
		_converter = ruler;
	}

	void Ruler::ConvertAll()
	{
		for (vector<Metric*>::iterator i = _metrics.begin(); i != _metrics.end(); i++)
		{
			Metric* metric = _converter->Find((*i)->GetName());
			
			if (metric)
			{
				map<Metric*, MetricValue> metrics = (*i)->GetValues();
				
				for (map<Metric*, MetricValue>::iterator j = metrics.begin(); j != metrics.end(); j++)
				{
					MetricValue result = metric->ConverTo(((*j).first)->GetName(), (*j).second);
					((*j)).first->Insert((*i), result);
				}
			}
			else
			{
				(*i)->Insert(0,0);
			}
		}
	}
	
	Metric* Ruler::CreateMetric(string name)
	{
		return new Metric(name);
	}

	Metric::Metric(string name)
	{
		_name = name;
	}
	Metric::~Metric()
	{
	}
	void Metric::Insert(Metric* m, MetricValue d)
	{
		_values[m] = d;
	}
	
	string Metric::GetName()
	{
		return _name;
	}
	
	MetricValue Metric::ConverTo(string name, MetricValue value, string ignore)
	{
		for (map<Metric*, MetricValue>::iterator i = _values.begin(); i != _values.end(); i++)
		{
			if (((*i).first)->GetName() != ignore)
			{
				if (((*i).first)->GetName() == name)
				{
					return (value * (*i).second);
				}
				else
				{
					MetricValue result = ((*i).first)->ConverTo(name, 1,  _name);
					
					if (result)
					{
						return (value * result * (*i).second);
					}
				}
			}
		}

		return 0;
	}
	
	map<Metric*, MetricValue> Metric::GetValues()
	{
		return _values;
	}
	
	/*
	    Parser of user input
	 */
	class Parser
	{
	private:
		Token _currentToken;
		
		string _name;
		MetricValue _value;

		std::istream* _input;
		
		Ruler* _ruler;
		Ruler* _finder;
		
		string		_u[2];
		MetricValue _v[2];
	
	public:
		Parser(std::istream*);
		Token GetToken();
		void Run();
		void SaveDataTo(Ruler*, Ruler*);

	};


/*
    Where stored result of parsing? In two rulers. 
    1st oringinal ruler, 
    2nd ruler, stored what result need to convert (find)
 */  
void Parser::SaveDataTo(Ruler* ruler, Ruler* finder)
{
	_ruler  = ruler;
	_finder = finder;
}

void Parser::Run()
{
	bool isFind = false;
	
	while(_input)
	{
		_currentToken = GetToken();
		
		if (_currentToken == END)
		{
			break;
		}
		else
		{
			switch (_currentToken)
			{
				
				case VALUE:
				{
					break;
				}
				case NAME:
				{
					break;
				}
				case ASSIGN:
				{
					_v[0] = _value;
					_u[0] = _name;
					break;
				}
				case SPACE:
				{
					break;
				}
				case QUESTION:
				{
					isFind = true;
					break;
				}
				
				case PRINT:
				{	
					_v[1] = _value;
					_u[1] = _name;
					
					Metric* metric1 = 0;
					Metric* metric2 = 0;
					
					if (isFind)
					{
						metric2 = _finder->Find(_u[1]);

						if (!metric2)
						{
						
							metric2 = _finder->CreateMetric(_u[1]);
							_finder->Insert(metric2);
						}
						
						metric1 = _finder->CreateMetric(_u[0]);
						metric2->Insert(metric1, _v[0]);
						
						isFind = false;
					}
					else
					{
						metric1 = _ruler->Find(_u[0]);
						metric2 = _ruler->Find(_u[1]);


						if (!metric1)
						{
							metric1 = _ruler->CreateMetric(_u[0]);
							_ruler->Insert(metric1);
						}

						if (!metric2)
						{
							metric2 = _ruler->CreateMetric(_u[1]);
							_ruler->Insert(metric2);
						}
						
						/* calculating values,
						   for example, if 10 seconds = 1 hour
						   culculate this 
						   1 seconds = 1/10 hour
						   1 hour = 10/1 seconds
						 */
						metric1->Insert(metric2, _v[0]/_v[1]);
						metric2->Insert(metric1, _v[1]/_v[0]);
					}
					
					break;
				}
			}
		}
	}
}

Parser::Parser(std::istream* i):
_input(i)
{}

Token Parser::GetToken()
{
	char ch = 0;

	_input->get(ch);

	switch (ch)
	{
		case 0:
		{
			return END;
		}
		case '=': case '?':
		{
			return Token(ch);
		}
		case '\n':
		{
			return PRINT;
		}
		case '0': case '1': case '2': case '3': case '4': case '5':
		case '6': case '7': case '8': case '9': case '.':
		{
			_input->putback(ch);
			(*_input) >> _value;
			return VALUE;
		}
		default:
		{
			if (isspace(ch))
			{
				return SPACE;
			}
		
			if (isalpha(ch))
			{
				_input->putback(ch);
				(*_input) >> _name;
				return NAME;
			}
			else
			{
				return PRINT;
			}
			break;
		}
	}
}


}
int main(int argc, char* argv[])
{
	Ruler::Ruler myRuler("My Favorite Ruler :-)");
	Ruler::Ruler myFindRuler("My Find Ruler!");
	
	myFindRuler.Using(&myRuler);
	
	Ruler::Parser parser = Ruler::Parser(&std::cin);
	parser.SaveDataTo(&myRuler, &myFindRuler);
	parser.Run();

	myFindRuler.ConvertAll();

	Ruler::Reporter reporter = Ruler::Reporter(&myFindRuler);
	
	/* set min, max values for user friendly output */
	reporter.Set(0.1, 1e6);
	reporter.Show();
}
