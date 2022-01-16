// test various C++ constructs, particularly wacky new ones.

#include <string>
#include <iostream>
#include <vector>

struct Person {
    std::string name;
    int id;
};

void fill_people(std::vector<Person>& people)
{
    Person guy{"Joe", 1}, gal{"Jane", 2};  // fancy initializing
    people.push_back(std::move(guy));
    people.push_back(std::move(gal));
}

int main()
{
    std::vector<Person> people;
    fill_people(people);

    for (auto peep : people) {
        std::cout << " #" << peep.id << ": " << peep.name << std::endl;
    }
}
