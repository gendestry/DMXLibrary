#include <iostream>
#include <iomanip>
#include <vector>
#define MAX_SIZE 100

std::string padByte(int num, int pad)
{
    std::string str = std::to_string(num);
    int len = str.length();
    if (len < pad)
    {
        str.insert(0, pad - len, ' ');
    }
    return str;
}

struct Fragment
{
    static unsigned int idCounter;
    static unsigned int fIdCounter;
    unsigned int id = 0;
    unsigned int fid = 0;
    int start = -1;

    Fragment()
    {
        id = idCounter++;
    }

    virtual Fragment *clone() const = 0;
    virtual unsigned int getSize() const = 0;
    virtual std::vector<uint8_t> getBytes() const = 0;

    virtual std::string toString() const
    {
        return "[ " + std::to_string(id) + " ]" + "Start: " + std::to_string(start) + " Size: " + std::to_string(getSize());
    }
};

unsigned int Fragment::idCounter = 0;
unsigned int Fragment::fIdCounter = 0;

struct FragmentedMemory
{
    std::vector<Fragment *> m_Fragments;
    unsigned int bytesPatched[MAX_SIZE] = {0};

    ~FragmentedMemory()
    {
        for (auto &f : m_Fragments)
        {
            delete f;
        }
    }

    void fillBytesPatched(int start, int end)
    {
        static unsigned int num = 1;
        for (int i = start; i < end; i++)
        {
            bytesPatched[i] = num;
        }

        num++;
    }

    bool add(Fragment &fragment, int start = -1)
    {

        if (m_Fragments.empty())
        {
            int size = fragment.getSize();
            if (size <= MAX_SIZE)
            {
                Fragment *f = fragment.clone();
                f->start = 0;
                m_Fragments.push_back(f);
                fillBytesPatched(0, size);
                return true;
            }
            else
            {
                std::cout << "Segment exceeds max size" << std::endl;
                return false;
            }
        }

        // insert next
        if (start == -1)
        {
            auto &frag = m_Fragments[m_Fragments.size() - 1];

            //     // if segment fits at end
            if (frag->start + frag->getSize() + fragment.getSize() <= MAX_SIZE)
            {
                Fragment *f = fragment.clone();
                f->start = frag->start + frag->getSize();
                m_Fragments.push_back(f);
                //         std::cout << frag << std::endl;
                //         // LightPatch *l = &segments.back().light;
                //         // std::cout << "L1: " << l << std::endl;
                //         // std::cout << "Adding segment: " << *l << std::endl;
                //         // ligthsByName[l->getName()].push_back(l);
                //         // LightPatch *l2 = ligthsByName[l->getName()].back();
                //         // std::cout << "L2: " << l2 << std::endl;

                //         // std::cout << l2->getName() << std::endl;

                //         // auto &newSeg = segments[segments.size() - 1];
                fillBytesPatched(f->start, f->start + f->getSize());
                return true;
            }
        }
        else
        {
            unsigned int size = fragment.getSize();
            for (int i = 0; i < m_Fragments.size(); i++)
            {

                auto &current = m_Fragments[i];
                // std::cout << "Current: " << current << std::endl;
                // std::cout << "i: " << i << ", Segment size: " << m_Fragments.size() << std::endl;

                // not last segment
                if (i < m_Fragments.size() - 1)
                {
                    // std::cout << "here" << std::endl;
                    auto &next = m_Fragments[i + 1];
                    // std::cout << "current: " << current->start << ", " << current->size << std::endl;
                    // std::cout << "next   : " << next->start << ", " << next->size << std::endl;
                    // std::cout << "other  : " << start << ", " << size << std::endl;
                    if (start >= current->start && start < current->start + current->getSize())
                    {
                        std::cout << "Segment within bounds" << std::endl;
                        return false;
                    }
                    // same for next
                    if (start >= next->start && start < next->start + next->getSize())
                    {
                        std::cout << "Segment within bounds" << std::endl;
                        return false;
                    }
                    if (current->start + current->getSize() <= start && start + size <= next->start)
                    {
                        Fragment *f = fragment.clone();
                        f->start = start;
                        m_Fragments.insert(m_Fragments.begin() + i + 1, f);
                        fillBytesPatched(start, start + size);
                        return true;
                    }
                    else
                    {
                        continue;
                    }
                }
                else
                {
                    // check if segment within current segment bounds
                    // std::cout << current->start << ", " << current->size << std::endl;
                    // std::cout << start << ", " << size << std::endl;
                    if (start >= current->start && start < current->start + current->getSize())
                    {
                        std::cout << "Segment within bounds 1" << std::endl;
                        return false;
                    }

                    if (start >= MAX_SIZE)
                    {
                        std::cout << "Segment start is out of bounds 2" << std::endl;
                        return false;
                    }

                    if (start + size <= MAX_SIZE)
                    {
                        Fragment *f = fragment.clone();
                        f->start = start;
                        m_Fragments.push_back(f);
                        // ligthsByName[light.getName()].push_back(&segments[segments.size() - 1].light);
                        fillBytesPatched(start, start + size);
                        return true;
                    }
                    else
                    {
                        std::cout << "Segment exceeds max size" << std::endl;
                        return false;
                    }
                }
            }
        }

        return false;
    }

    std::vector<uint8_t> getBytes() const
    {
        std::vector<uint8_t> bytes(MAX_SIZE, 0);
        int index = 0;
        for (int i = 0; i < m_Fragments.size(); i++)
        {
            auto &curr = m_Fragments[i];
            const auto &currBytes = curr->getBytes();
            for (int j = curr->start; j < curr->start + curr->getSize(); j++)
            {
                bytes[j] = currBytes[j - curr->start];
            }
        }

        return bytes;
    }

    std::vector<Fragment *> getFragmentsById(unsigned int id) const
    {
        std::vector<Fragment *> fragments;
        for (auto &f : m_Fragments)
        {
            if (f->id == id)
            {
                fragments.push_back(f);
            }
        }

        return fragments;
    }

    std::vector<Fragment *> operator[](unsigned int id)
    {
        return getFragmentsById(id);
    }

    void printFragments()
    {
        // std::cout << "Segments: " << segments.size() << std::endl;
        for (int i = 0; i < m_Fragments.size(); i++)
        {
            const int offset = 0;
            auto &curr = m_Fragments[i];
            int cstart = curr->start;
            int cnext = curr->start + curr->getSize();
            int cend = curr->start + curr->getSize() - 1;

            if (cstart != 0 && i == 0)
            {
                std::cout << "[" << std::setw(3) << 0 + offset << ", " << std::setw(3) << (cstart - 1 + offset) << "]" << " Empty" << std::endl;
            }

            std::cout << "[" << std::setw(3) << (curr->start + offset) << ", " << std::setw(3) << (curr->start + curr->getSize() - 1 + offset) << "] " << curr->toString() << std::endl;

            if (i < m_Fragments.size() - 1)
            {
                auto &next = m_Fragments[i + 1];
                if (curr->start + curr->getSize() != next->start)
                {
                    std::cout << "[" << std::setw(3) << (curr->start + curr->getSize() + offset) << ", " << std::setw(3) << (next->start - 1 + offset) << "]" << " Empty" << std::endl;
                    continue;
                }
            }
            else
            {
                if (curr->start + curr->getSize() < MAX_SIZE)
                {
                    std::cout << "[" << std::setw(3) << (curr->start + curr->getSize() + offset) << ", " << std::setw(3) << (MAX_SIZE - 1 + offset) << "]" << " Empty" << std::endl;
                    continue;
                }
            }
        }
    }

    void printFragmentsWithId(unsigned int id)
    {
        std::vector<Fragment *> fragments = getFragmentsById(id);

        for (int i = 0; i < fragments.size(); i++)
        {
            const int offset = 0;
            auto &curr = fragments[i];
            std::cout << "[" << std::setw(3) << (curr->start + offset) << ", " << std::setw(3) << (curr->start + curr->getSize() - 1 + offset) << "] " << curr->toString() << std::endl;
        }
    }

    void printBytes() const
    {

        // const std::string color2 = "\x1B[38;2;0;110;70m";
        // const std::string color = "\x1B[38;2;60;240;180m";
        const std::string color = "\x1B[32m";
        const std::string color2 = "\x1B[33m";
        const std::string reset = "\x1B[0m";
        std::vector<uint8_t> bytes = getBytes();
        std::cout << "Bytes: \n";

        unsigned int prevByteColor = 0;
        bool colorToggle = false;
        std::string col = reset;

        std::cout << "\x1B[3m";

        int cond = std::min((int)bytes.size(), 16);
        for (int i = 0; i < cond; i++)
        {
            static const std::string hex = "0123456789ABCDEF";
            std::cout << "0x" << hex[i] << "  ";
        }

        std::cout << std::dec << reset << std::endl;

        for (int i = 0; i < cond; i++)
        {
            std::cout << "----";
            if (i < cond - 1)
                std::cout << "-";
        }

        for (int i = 0; i < bytes.size(); i++)
        {
            if (i % 16 == 0)
                std::cout << std::endl;

            if (bytesPatched[i] != 0)
            {
                if (prevByteColor != bytesPatched[i])
                {
                    colorToggle = !colorToggle;
                    prevByteColor = bytesPatched[i];
                    col = colorToggle ? color : color2;
                }
            }
            else
            {
                col = reset + "\x1B[2m" + "\x1B[3m";
            }
            std::cout << col << padByte(bytes[i], 3) << reset << "  ";
        }

        std::cout << std::endl;
    }
};

struct Col : public Fragment
{
    uint8_t r, g, b;
    std::string name;

    Col() = default;
    Col(uint8_t r, uint8_t g, uint8_t b, std::string name = "RGB") : r(r), g(g), b(b), name(name) {}

    Fragment *clone() const override
    {
        return new Col(*this);
    }

    unsigned int getSize() const override
    {
        return 3;
    }

    std::vector<uint8_t> getBytes() const override
    {
        return {r, g, b};
    }

    std::string toString() const override
    {
        return Fragment::toString() + ": " + name + "(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ")";
    }

    friend std::ostream &operator<<(std::ostream &os, const Col &col)
    {
        os << col.toString();
        return os;
    }
};

struct LedPAR : public Fragment
{
    uint8_t r, g, b, dim, strobo;
    std::string name = "LedPAR";

    LedPAR() = default;
    LedPAR(uint8_t r, uint8_t g, uint8_t b, uint8_t dim, uint8_t strobo) : r(r), g(g), b(b), dim(dim), strobo(strobo) {}

    Fragment *clone() const override
    {
        return new LedPAR(*this);
    }

    unsigned int getSize() const override
    {
        return 5;
    }

    std::vector<uint8_t> getBytes() const override
    {
        return {r, g, b, dim, strobo};
    }

    std::string toString() const
    {
        return Fragment::toString() + ": " + name + "(" + std::to_string(r) + ", " + std::to_string(g) + ", " + std::to_string(b) + ", " + std::to_string(dim) + ", " + std::to_string(strobo) + ")";
    }

    friend std::ostream &operator<<(std::ostream &os, const LedPAR &par)
    {
        os << par.toString();
        return os;
    }
};

int main()
{

    FragmentedMemory memory;
    Col red = {255, 127, 12, "red"};
    if (!memory.add(red))
    {
        return -1;
    }

    red.g = 33;

    if (!memory.add(red, 5))
    {
        return -1;
    }

    LedPAR par = {255, 0, 0, 0, 0};

    if (!memory.add(par, 10))
    {
        return -1;
    }

    par.g = 127;

    if (!memory.add(par))
    {
        return -1;
    }

    par.b = 255;
    if (!memory.add(par))
    {
        return -1;
    }

    par.dim = 100;
    if (!memory.add(par))
    {
        return -1;
    }

    memory.printFragments();
    memory.printBytes();
    // memory.printFragmentsWithId(1);

    // memory[id][fields]
    /*
        struct fieldColor {
            uint8_t r, g, b;
        }
     */

    // TODO: how do this
    // auto pars = memory[1];
    // for(auto &p: pars) {
    //     pars[0]->g = 255;
    // }
    // memory.add(1, red);
}