import React, {useState, useEffect} from "react";
import {Container, Form, Button, ModalHeader, ModalBody, ModalFooter} from 'react-bootstrap';
import logo from '../Assets/logo.PNG';
import axios from "axios";
import { useContextApi } from "../ContexApi";
import { useNavigate } from "react-router-dom";
import { ToastContainer, toast } from 'react-toastify';
import { Modal } from "react-bootstrap";
import { AdapterDayjs } from '@mui/x-date-pickers/AdapterDayjs';
import { LocalizationProvider } from '@mui/x-date-pickers/LocalizationProvider';
import { DateTimePicker } from '@mui/x-date-pickers/DateTimePicker';
import dayjs from "dayjs";
export default function Home(){

    const {token, setToken, login, setLogin, userData} = useContextApi();
    const navigate = useNavigate();
    const [calendar, setCalendar] = useState([]);
    const [weeks, setWeeks] = useState([]);
    const [events, setEvents] = useState([]);
    const [eventsInCurrentMonth, setEventsInCurrentMonth] = useState({});
    const [openAddMod, setOpenAddMod] = useState(false);
    const [handleAddEventDetails, sethandleAddEventDetails] = useState({
        title: "",
        description: "",
        dateTime: dayjs(new Date()),
        dateTimeISO: dayjs(new Date()).toISOString()
    });
    const [selectedDate, setSelectedDate] = useState(new Date().getDate());

    const daysInMonth = (month, year) => new Date(year, month, 0).getDate();
    const getFirstDayOfMonth = (month, year) => new Date(year, month - 1, 1).getDay();

    const getColors = (eventNum) => {
        if (eventsInCurrentMonth[eventNum]?.length == 0){
            return "#4DE938";
        }
        else if (eventsInCurrentMonth[eventNum]?.length <= 2){
            return "#FFDD00";
        }
        else {
            return "#FF6666";
        }

    }

    const generateCalendar = (month, year) => {
        const totalDays = daysInMonth(month, year);
        const firstDay = getFirstDayOfMonth(month, year); // Sunday = 0, Monday = 1, ...
        let daysArray = Array.from({ length: firstDay }, () => ""); // Empty placeholders
        for (let i = 1; i <= totalDays; i++) {
          daysArray.push(i);
        }
        return daysArray;
    };

    const returnWeeksArray = (daysArray) => {
        let weeksArray = [];
        let tempArray = [];
        for (let i=0; i<daysArray.length; i++){
            tempArray.push(daysArray[i]);
            if (tempArray.length === 7){
                weeksArray.push(tempArray);
                tempArray = [];
            }
        }
        if (tempArray.length > 1){
            weeksArray.push(tempArray);
        }
        return weeksArray;
    }


    const fetchEvents = () => {
        if (login) {
            axios.get("http://localhost:2028/api/events/", {
                headers: {
                    "Authorization": `Bearer ${token}`
                }
            }).then((resp) => {
                setEvents(resp.data);
                console.log(resp.data);
            })
        }
    }

    const eventsByDate = () =>{
        let tempEvents = {};
        let eventDate = "";
        for (let i=1; i<=daysInMonth(new Date().getMonth()+1, (new Date).getFullYear());i++){
            tempEvents[i] = [];
        }
        for (let i=0; i<events.length; i++){
            eventDate = new Date(events[i].dateTime);
            if (eventDate.getMonth() == new Date().getMonth() && eventDate.getFullYear() == new Date().getFullYear()){
                tempEvents[eventDate.getDate()].push(events[i]);
            }
        }
        console.log(tempEvents);
        setEventsInCurrentMonth(tempEvents);
    }

    const handleAddEvent = () => {
        axios.post("http://localhost:2028/api/events/", {
            name: handleAddEventDetails.title,
            description: handleAddEventDetails.description,
            dateTime: handleAddEventDetails.dateTimeISO
        }, {
            headers: {
                "Authorization": `Bearer ${token}`
            }
        }).then((resp) => {
            console.log(resp);
            fetchEvents();
            setOpenAddMod(false);
        }).catch((err) => {
            console.log(err);
            setOpenAddMod(false);
        })
    }

    useEffect(() => {
        if (!login) {
            navigate("/login")
        }
        let tempCalendar = generateCalendar(new Date().getMonth()+1, new Date().getFullYear());
        setCalendar(tempCalendar);
        let tempWeeks = returnWeeksArray(tempCalendar);
        setWeeks(tempWeeks);
        fetchEvents();
    }, [login]);
    
    useEffect(() => {
        eventsByDate();
    }, [events]);

    
    return(
        <div style={{display:'flex', alignItems:'stretch', height:'100%'}}>
            <Container className="sidebar" style={{maxWidth: "100%", backgroundColor: "#2463AB", width: "20%", padding: '0 !important', margin:'0 !important'}}>
                <p className="h2 text-center mt-5" style={{color:'white'}}>Navigation</p>
                <Container className="sidebar-links">
                    <p className="nav-btn text-center active">Home</p>
                    <p className="nav-btn text-center">Aggregate Calculator</p>
                    <p className="nav-btn text-center">Group Former</p>
                    <p className="nav-btn text-center">Quiz Bank</p>
                    <p className="nav-btn text-center">Your Profile</p>
                    <p className="nav-btn text-center" onClick={() => {setToken(null); localStorage.removeItem("token"); setLogin(false); navigate("/login")}}><strong>Logout</strong></p>
                </Container>
            </Container>
            <Container className="main" style={{padding:'0', margin:'0', minHeight:'100vh'}}>
                <Container className="header" style={{width:'100%', border:'1px solid black', padding: '0', margin:'0', minHeight:'15%'}}>
                    <Container style={{padding: '20px', textAlign:'center', display:'flex', alignItems:'center', justifyContent:'center'}}>
                        <img src={logo} style={{width:'50px', height:'50px', marginRight:'10px'}} alt="logo" />
                        <p className="h1">StudentSync</p>
                    </Container>
                </Container>
                <Container className="mainContent" style={{backgroundColor:'#D4D4D4', minHeight:'85%',display:'flex',flexDirection:'column'}}>
                    <Container style={{marginTop:'50px'}}>
                        <h2 className="text-center">Task Calendar</h2>
                        <h4 className="text-center h4">({new Date().toLocaleString('default', { month: 'long' })} {new Date().getFullYear()})</h4>
                        <Container style={{width:'80%'}}>
                            {userData?.isAdmin &&
                            <Container className="admin-btns" style={{display:'flex', justifyContent:'flex-end'}}>
                                <Button onClick={()=>{setOpenAddMod(true)}}>
                                    <span>
                                        +
                                    </span>
                                </Button>
                            </Container>}
                            <Container style={{marginTop:'20px', display:'flex', flexDirection:'column', justifyContent:'center', backgroundColor:'white', borderRadius:'20px'}}>
                                <Container className="days-row" style={{display:'flex', justifyContent:'space-around', width:'100%', padding:'10px'}}>
                                    <p className="days">Sun</p>                                
                                    <p className="days">Mon</p>
                                    <p className="days">Tue</p>
                                    <p className="days">Wed</p>
                                    <p className="days">Thu</p>
                                    <p className="days">Fri</p>
                                    <p className="days">Sat</p>
                                </Container>
                                <table className="dates">
                                    <tbody>
                                        {
                                            weeks?.map((week)=>{
                                                return (
                                                    <tr className="dates-row">
                                                        {
                                                            week.map((day)=>{
                                                                return <td className="date" onClick={() => {setSelectedDate(day)}} style={{backgroundColor: getColors(day)}}>{day}</td>
                                                            })
                                                        }
                                                    </tr>
                                                )
                                            })
                                        }
                                    </tbody>
                                </table>
                        
                            </Container>
                        </Container>
                        <p className="text-muted text-center mt-3 mb-0">(Click the day to view the scheduled tasks.)</p>
                    </Container>
                    <Container className="bottom-container" style={{marginBottom:'10px'}}>
                        <Container className="colors" style={{marginBottom:'50px'}}>
                            <Container style={{display:'flex', alignItems:'center'}}>
                                <Container className="color" style={{backgroundColor:'#4DE938'}}>
                                </Container>
                                <span style={{marginLeft:'10px'}}>No Tasks</span>
                            </Container>
                            <Container style={{display:'flex', alignItems:'center'}}>
                                <Container className="color" style={{backgroundColor:'#FFDD00'}}>
                                </Container>
                                <span style={{marginLeft:'10px'}}>&lt;= 2 tasks</span>
                            </Container>
                            <Container style={{display:'flex', alignItems:'center'}}>
                                <Container className="color" style={{backgroundColor:'#FF6666'}}>
                                </Container>
                                <span style={{marginLeft:'10px'}}>&gt;= 3 tasks</span>
                            </Container>
                        </Container>
                        <Container className="events" style={{width:'50%', marginBottom:'50px'}}>
                            <p className="h6">Scheduled Tasks/Event</p>
                            <ul className="events-list">    
                                {
                                    eventsInCurrentMonth[selectedDate]?.map((event) => {
                                        return (
                                                <li className="event">
                                                    <strong>
                                                        {event.name} -- {new Date(event.dateTime).toLocaleString('en-US', { hour: 'numeric', minute: 'numeric', hour12: true })
                                                    }
                                                    </strong>
                                                </li>
                                        )
                                    })
                                }
                            </ul>
                        </Container>
                    </Container>
                
                    <Container className="modals-container">
                        <Modal show={openAddMod} onHide={() => {setOpenAddMod(false)}}>
                            <ModalHeader closeButton>
                                <h2 className="h5 text-center">Add Event</h2>
                            </ModalHeader>
                            <ModalBody>
                                <Form>
                                    <Form.Group className="mb-3">
                                        <Form.Label>Event DateTime</Form.Label>
                                        <Container className="date-time-container" style={{margin:0, padding:0, width:'100%'}}>
                                            <LocalizationProvider dateAdapter={AdapterDayjs}>
                                                <DateTimePicker value={handleAddEventDetails.dateTime} onChange={(e) => {sethandleAddEventDetails({...handleAddEventDetails, dateTime: e, dateTimeISO: e.toISOString()})}}/>
                                            </LocalizationProvider>                                
                                        </Container>
                                    </Form.Group>
                                    <Form.Group className="mb-3">
                                        <Form.Label>Event Name</Form.Label>
                                        <Form.Control type="text" required placeholder="Enter Event Name" value={handleAddEventDetails.title} onChange={(e) => {sethandleAddEventDetails({...handleAddEventDetails, title: e.target.value})}}/>
                                    </Form.Group>
                                    <Form.Group className="mb-3">
                                        <Form.Label>Event Description</Form.Label>
                                        <Form.Control as="textarea" required placeholder="Enter Event Description" value={handleAddEventDetails.description} onChange={(e) => {sethandleAddEventDetails({...handleAddEventDetails, description: e.target.value})}}/>
                                    </Form.Group>
                                </Form>
                            </ModalBody>
                            <ModalFooter>
                                <Button variant="secondary" onClick={()=>setOpenAddMod(false)}>Close</Button>
                                <Button variant="primary" onClick={handleAddEvent}>Add Event</Button>
                            </ModalFooter>
                        </Modal>
                    </Container>
                </Container>
            </Container>
        </div>
    )
}